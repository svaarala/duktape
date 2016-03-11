/*
 *  Testrunner server.
 */

var fs = require('fs');
var yaml = require('yamljs');
var express = require('express');
var bodyParser = require('body-parser');
var Datastore = require('nedb');
var https = require('https');
var GitHubApi = require('github');
var Promise = require('bluebird');
var RateLimiter = require('limiter').RateLimiter;

var util = require('./util');
var assert = util.assert;
var expressutil = require('./expressutil');
var dbutil = require('./dbutil');
var githubutil = require('./githubutil');
var webuiutil = require('./webuiutil');

function main() {
    console.log('Duktape testrunner server');

    // Parse args and config, initialize a shared 'state' object.
    var argv = require('minimist')(process.argv.slice(2));
    console.log('Command line options: ' + JSON.stringify(argv));
    var configFile = argv.config || './config.yaml';
    console.log('Load config: ' + configFile);
    var state = yaml.load(configFile);

    // Datastore
    console.log('Initialize NeDB datastore: ' + state.databaseFile);
    var db = new Datastore({
        filename: state.databaseFile,
        autoload: true
    });
    state.db = db;
    //dbutil.deleteAll(db);

    // Github API instance.
    var github = new GitHubApi({
        version: '3.0.0',
        debug: false,
        protocol: 'https',
        host: 'api.github.com',
        timeout: 5000,
        header: {
            'user-agent': 'duktape-testrunner'
        }
    });
    state.github = github;
    state.githubLimiter = new RateLimiter(assert(state.githubTokensPerHour), 'hour', true);  // sanity limit

    // Express and shared helpers.
    var app = express();
    var apiJsonBodyParser = expressutil.makeApiJsonBodyParser();
    var githubJsonBodyParser = expressutil.makeGithubJsonBodyParser(state.githubWebhookSecret);
    var apiBasicAuth = expressutil.makeApiBasicAuth(state.clientAuthUsername, state.clientAuthPassword, state.serverAuthPassword);
    var logRequest = expressutil.makeLogRequest();

    // URI paths served.
    app.get('/', logRequest,
            webuiutil.makeIndexHandler(state));
    app.get('/out/:sha', logRequest,
            webuiutil.makeDataFileHandler(state));
    app.post('/github-webhook', logRequest, githubJsonBodyParser,
             githubutil.makeGithubWebhookHandler(state));
    app.post('/get-commit-simple', logRequest, apiBasicAuth, apiJsonBodyParser,
             githubutil.makeGetCommitSimpleHandler(state));
    app.post('/finish-commit-simple', logRequest, apiBasicAuth, apiJsonBodyParser,
             githubutil.makeFinishCommitSimpleHandler(state));

    // HTTPS server.
    var apiServer = https.createServer({
        key: fs.readFileSync(state.serverPrivateKey),
        cert: fs.readFileSync(state.serverCertificate)
    }, app);
    apiServer.listen(state.serverPort, function () {
        var host = apiServer.address().address;
        var port = apiServer.address().port;
        console.log('Duktape testrunner server listening at https://%s:%s', host, port);
    });

    // Background job for hanging request timeouts, persistent
    // Github pushes, etc.
    function periodicDatabaseScan() {
        githubutil.pushGithubStatuses(state);       // persistent github status pushing
        githubutil.handleGetCommitRequests(state);  // webhook client timeouts
    }
    var dbScanTimer = setInterval(periodicDatabaseScan, 5000);
}
main();
