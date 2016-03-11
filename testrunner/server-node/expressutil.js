/*
 *  Express.js utilities.
 */

var bodyParser = require('body-parser');
var basicAuth = require('basic-auth');
var crypto = require('crypto');

var util = require('./util');
var sha1sum = util.sha1sum;
var sha1sumFile = util.sha1sumFile;
var assert = util.assert;

// Express.js API JSON body parser.
// http://stackoverflow.com/questions/19917401/node-js-express-request-entity-too-large
var makeApiJsonBodyParser = function makeApiJsonBodyParse() {
    return bodyParser.json({
        limit: '50mb'
    });
};

// Express.js Github JSON body parser with github auth.
function makeGithubJsonBodyParser(webhookSecret) {
    return bodyParser.json({
        limit: '50mb',
        verify: function (req, res, buf, encoding) {
            function authFail() {
                console.log(req.method + ' ' + req.url + ': github authentication failure');
                throw new Error('github authentication failure');
            }
            if (typeof webhookSecret !== 'string') {
                authFail();
            }
            var hubSig = req.get('X-Hub-Signature');
            if (typeof hubSig !== 'string') {
                authFail();
            }
            var mac = crypto.createHmac('sha1', assert(webhookSecret));
            mac.update(buf);
            var compare = 'sha1=' + mac.digest('hex').toLowerCase();
            if (hubSig !== compare) {
                authFail();
            }
        }
    });
}

// Express.js API authentication for testrunner API calls.
function makeApiBasicAuth(clientUsername, clientPassword, serverPassword) {
    return function (req, res, next) {
        function unauthorized(res) {
            console.log(req.method + ' ' + req.url + ': api authentication failed');
            res.set('WWW-Authenticate', 'Basic realm=API Authorization Required');
            return res.sendStatus(401);
        }
        var user = basicAuth(req);
        if (!user || !user.name || !user.pass) {
            return unauthorized(res);
        }
        if (user.name === assert(clientUsername) &&
            user.pass === assert(clientPassword)) {
            res.set('X-TestRunner-Authenticator', assert(serverPassword));
            return next();
        } else {
            return unauthorized(res);
        }
    };
}

// Express.js request logger.
function makeLogRequest() {
    return function (req, res, next) {
        console.log(req.method + ' ' + req.url);
        next();
    };
}

exports.makeApiJsonBodyParser = makeApiJsonBodyParser;
exports.makeGithubJsonBodyParser = makeGithubJsonBodyParser;
exports.makeApiBasicAuth = makeApiBasicAuth;
exports.makeLogRequest = makeLogRequest;
