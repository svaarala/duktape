/*
 *  Testrunner client for simple commit jobs
 *
 *  Node.js based default client intended for Linux, OSX, and Windows.
 */

var fs = require('fs');
var path = require('path');
var yaml = require('yamljs');
var Promise = require('bluebird');
var http = require('http');
var https = require('https');
var tmp = require('tmp');
var child_process = require('child_process');
var crypto = require('crypto');

/*
 *  Command line and config parsing
 */

var argv = require('minimist')(process.argv.slice(2));
console.log('Command line options: ' + JSON.stringify(argv));
var configFile = argv.config || './config.yaml';
console.log('Load config: ' + configFile);
var clientConfig = yaml.load(configFile);

/*
 *  Misc utils
 */

function sha1sum(x) {
    return crypto.createHash('sha1').update(x).digest('hex');
}

function sha1sumFile(x) {
    return sha1sum(fs.readFileSync(x));
}

function assert(x) {
    if (x) { return x; }
    throw new Error('assertion failed');
}

/*
 *  HTTP(S) helpers
 */

var serverTrustRoot = fs.readFileSync(clientConfig.serverCertificate);

// Path -> file data
var fileCache = {};

function getBasicAuthHeader() {
    return 'Basic ' + new Buffer(assert(clientConfig.clientAuthUsername) + ':' + assert(clientConfig.clientAuthPassword)).toString('base64');
}

function getUserAgent() {
    return 'testrunner-client ' + clientConfig.clientName;
}

function serverAuthCheck(res) {
    var auth = res.headers['x-testrunner-authenticator'];  // lowercase intentionally
    if (typeof auth !== 'string' ||
        auth !== assert(clientConfig.serverAuthPassword)) {
        throw new Error('server not authorized');
    }
}

function postJson(path, request, cb) {
    return new Promise(function (resolve, reject) {
        var requestData = new Buffer(JSON.stringify(request), 'utf8');
        var headers = {
            'User-Agent': getUserAgent(),
            'Authorization': getBasicAuthHeader(),
            'Content-Type': 'application/json',
            'Content-Length': requestData.length
        };
        var options = {
            host: clientConfig.serverHost,
            port: clientConfig.serverPort,
            ca: serverTrustRoot,
            rejectUnauthorized: true,
            path: path,
            method: 'POST',
            auth: assert(clientConfig.clientAuthUsername) + ':' + assert(clientConfig.clientAuthPassword),
            headers: headers
        };

        console.log('sending POST ' + path);

        var req = https.request(options, function (res) {
            //res.setEncoding('utf8');  // we just want binary

            var buffers = [];
            res.on('data', function (data) {
                try {
                    buffers.push(data);
                } catch (e) {
                    console.log(e);
                }
            });
            res.on('end', function () {
                try {
                    serverAuthCheck(res);
                    var data = Buffer.concat(buffers);
                    var rep = JSON.parse(data.toString('utf8'));
                } catch (e) {
                    reject(e);
                    return;
                }
                resolve(rep);
            });
        });

        req.on('error', function (err) {
            reject(err);
        });

        req.write(requestData);
        req.end();
    });
}

function getFile(path, cb) {
    return new Promise(function (resolve, reject) {
        var headers = {
            'User-Agent': getUserAgent(),
            'Authorization': getBasicAuthHeader()
        };
        var options = {
            host: clientConfig.serverHost,
            port: clientConfig.serverPort,
            ca: serverTrustRoot,
            rejectUnauthorized: true,
            path: path,
            method: 'GET',
            auth: assert(clientConfig.clientAuthUsername) + ':' + assert(clientConfig.clientAuthPassword),
            headers: headers
        };

        console.log('sending GET ' + path);

        var req = https.request(options, function (res) {
            //res.setEncoding('utf8');  // we just want binary

            var buffers = [];
            res.on('data', function (data) {
                try {
                    buffers.push(data);
                } catch (e) {
                    console.log(e);
                }
            });
            res.on('end', function () {
                try {
                    serverAuthCheck(res);
                    var data = Buffer.concat(buffers);
                } catch (e) {
                    reject(e);
                    return;
                }
                resolve(data);
            });
        });

        req.on('error', function (err) {
            reject(err);
        });

        req.end();
    });
}

function cachedGetFile(path) {
    if (fileCache[path]) {
        return new Promise(function (resolve, reject) {
            resolve(fileCache[path]);
        });
    } else {
        return getFile(path).then(function (data) {
            fileCache[path] = data;
            return data;
        });
    }
}

/*
 *  Simple commit jobs
 */

function processSimpleScriptJob(rep) {
    var context, repo, repoFull, repoCloneUrl, sha, scriptcmd;
    var tmpDir;

    var tmpDir = tmp.dirSync({
        mode: 0750,
        prefix: 'testrunner-' + (rep.context || 'undefined') + '-',
        unsafeCleanup: true
    });

    return new Promise(function (resolve, reject) {
        var i;

        context = assert(rep.context);
        repo = assert(rep.repo);
        repoFull = assert(rep.repo_full);
        repoCloneUrl = assert(rep.repo_clone_url);
        sha = assert(rep.sha);
        fetch_ref = rep.fetch_ref;  // optional, only for pulls

        for (i = 0; i < clientConfig.supportedContexts.length; i++) {
            if (clientConfig.supportedContexts[i].context === assert(rep.context)) {
                scriptcmd = clientConfig.supportedContexts[i].command;
            }
        }
        if (!scriptcmd) {
            reject(new Error('unsupported context: ' + context));
            return;
        }

        console.log('start simple commit test, repo ' + repoCloneUrl + ', sha ' + sha + ', context ' + context);

        var args = [].concat(scriptcmd);
        args = args.concat([
            '--repo-full-name', assert(repoFull),
            '--repo-clone-url', assert(repoCloneUrl),
            '--commit-name', assert(sha),
            '--context', assert(context),
            '--temp-dir', assert(tmpDir.name),
            '--repo-snapshot-dir', assert(clientConfig.repoSnapshotDir)
        ]);
        if (fetch_ref) {
            args = args.concat([
                '--fetch-ref', fetch_ref
            ]);
        }

        // XXX: child timeout and recovery
        var cld = child_process.spawn(args[0], args.slice(1), { cwd: '/tmp' });
        var buffers = [];
        cld.stdout.on('data', function (data) {
            buffers.push(data);
        });
        cld.stderr.on('data', function (data) {
            buffers.push(data);  // just interleave stdout/stderr for now
        });
        cld.on('close', function (code) {
            resolve({ code: code, data: Buffer.concat(buffers) });
        });
    }).then(function (res) {
        // Test execution finished, test case may have succeeded or failed.
        console.log('finished simple commit test, repo ' + repoCloneUrl + ', sha ' + sha + ', context ' + context + ', code ' + res.code);
        //console.log(res.data.toString());

        // Initial heuristic for providing descriptions from test script,
        // could also use an explicit output file.  Match last occurrence
        // on purpose so that the script can always override a previous
        // status if necessary.
        var desc = null;
        try {
            desc = (function () {
                var txt = res.data.toString('utf8');  // XXX: want something lenient
                var re = /^TESTRUNNER_DESCRIPTION: (.*?)$/gm;
                var m;
                var out = null;
                while ((m = re.exec(txt)) !== null) {
                    out = m[1];
                }
                return out;
            })();
        } catch (e) {
            console.log(e);
        }

        postJson('/finish-commit-simple', {
            repo: assert(repo),
            repo_full: assert(repoFull),
            repo_clone_url: assert(repoCloneUrl),
            sha: assert(sha),
            context: assert(context),
            state: res.code === 0 ? 'success' : 'failure',
            description: desc || (res.code === 0 ? 'Success' : 'Failure'),
            text: res.data.toString('base64')
        }).then(function (rep) {
            //console.log(rep);
        }).catch(function (err) {
            console.log(err);  // XXX: chain error
        });
    }).catch(function (err) {
        // Test execution attempt failed (which is different from a test case
        // failing).
        console.log('FAILED simple commit test, repo ' + repoCloneUrl + ', sha ' + sha + ', context ' + context);
        console.log(err);

        var data = new Buffer(String(err.stack || err));

        // XXX: indicate possibly transient nature of error

        postJson('/finish-commit-simple', {
            client_name: clientConfig.clientName,
            repo: assert(repo),
            repo_full: assert(repoFull),
            repo_clone_url: assert(repoCloneUrl),
            sha: assert(sha),
            context: assert(context),
            state: 'failure',
            description: 'Failed: ' + String(err),
            text: data.toString('base64')
        }).then(function (rep) {
            //console.log(rep);
        }).catch(function (err) {
            console.log(err);  // XXX: chain error
        });
    }).finally(function () {
        if (tmpDir) {
            tmpDir.removeCallback();
            console.log('cleaned up ' + tmpDir.name);
        }
    });
}

function requestAndExecute() {
    var contexts = [];
    var i;

    for (i = 0; i < clientConfig.supportedContexts.length; i++) {
        contexts.push(clientConfig.supportedContexts[i].context);
    }

    postJson('/get-commit-simple', {
        client_name: clientConfig.clientName,
        contexts: contexts
    }).then(function (rep) {
        console.log(rep);
        if (rep.error_code) {
            throw new Error('job failed; error code ' + rep.error_code + ': ' + rep.error_description);
        }
        return processSimpleScriptJob(rep);
    }).then(function () {
        setTimeout(function () { requestAndExecute(); }, clientConfig.sleepJobSuccess);
    }).catch(function (err) {
        // XXX: differentiate between TIMEOUT and other errors
        console.log('job failed; sleep a bit and retry:', err);
        console.log(err.stack);
        setTimeout(function () { requestAndExecute(); }, clientConfig.sleepJobFailure);
    });

    setTimeout(function () {}, 10000000);  // avoid exit
}

function main() {
    requestAndExecute();
}

main();
