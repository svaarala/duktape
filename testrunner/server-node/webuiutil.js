/*
 *  Web UI helpers.
 */

var fs = require('fs');
var path = require('path');
var Promise = require('bluebird');

var dbutil = require('./dbutil');
var util = require('./util');
var assert = util.assert;

function makeIndexHandler(state) {
    return function (req, res) {
        var out = [];

        // XXX: replace with an actually useful status page, proper html generation
        // XXX: eventually allow commit jobs to be started/restarted

        Promise.resolve().then(function () {
            out.push('<html>');
            out.push('<head>');
            out.push('<title>Duktape testrunner</title>');
            out.push('</head>');
            out.push('<body>');
            out.push('<h1>Duktape testrunner</h1>');

            return dbutil.find(state.db, {
                type: 'commit_simple'
            });
        }).then(function (docs) {
            out.push('<h2>Simple jobs</h2>');
            out.push('<table>');
            out.push('<tr><th>Repo</th><th>Commit hash</th><th>Committer</th><th>Runs</th></tr>');
            docs.forEach(function (d) {
                out.push('<tr>');
                out.push('<td>' + d.repo_full + '</td>');
                out.push('<td>' + d.sha + '</td>');
                out.push('<td>' + d.committer + '</td>');
                out.push('<td>');
                d.runs.forEach(function (r) {
                    out.push(' ' + r.context + '/' + r.start_time + '/' + r.end_time);
                });
                out.push('</td>');
                out.push('</tr>');
            });
            out.push('</table>');

            return dbutil.find(state.db, {
                type: 'github_status'
            });
        }).then(function (docs) {
            out.push('<h2>Github statuses</h2>');
            out.push('<table>');
            out.push('<tr><th>Repo</th><th>Commit hash</th><th>Context</th><th>State</th><th>Description</th><th>Target URL</th><th>Dirty</th></tr>');
            docs.forEach(function (d) {
                out.push('<tr>');
                out.push('<td>' + d.repo + '</td>');
                out.push('<td>' + d.sha + '</td>');
                out.push('<td>' + d.context + '</td>');
                out.push('<td>' + d.state + '</td>');
                out.push('<td>' + d.description + '</td>');
                out.push('<td>' + d.target_url + '</td>');
                out.push('<td>' + d.dirty + '</td>');
                out.push('</tr>');
            });
            out.push('</table>');
        }).then(function () {
            out.push('</body>');
            out.push('</html>');
        }).then(function () {
            res.send(out.join(''));
        }).catch(function (err) {
            res.send('ERROR: ' + err);
        });
    }
}

function makeDataFileHandler(state) {
    return function (req, res) {
        Promise.resolve().then(function () {
            assert(req.params);
            var sha = req.params.sha;
            assert(sha);

            // minimal validation, also ensures no '..' etc.
            if (!/^[0-9a-f$]+/.test(sha)) {
                throw new Error('invalid URI: ' + req.url);
            }

            var fn = path.join(assert(state.dataDumpDirectory), sha);
            var data = fs.readFileSync(fn);
            res.setHeader('content-type', 'text/plain; charset=utf-8');
            res.send(data);
        }).catch(function (err) {
            res.status(500);
            res.send('ERROR: ' + err);
        });
    }
}

exports.makeIndexHandler = makeIndexHandler;
exports.makeDataFileHandler = makeDataFileHandler;
