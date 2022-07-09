'use strict';

const { createBareObject } = require('../util/bare');
const { tokenizeFiles } = require('../util/tokenize');

const tokenizeCommandSpec = createBareObject({
    description: 'Tokenize text files',
    options: createBareObject({})
});
exports.tokenizeCommandSpec = tokenizeCommandSpec;

function tokenizeCommand({ commandPositional }, autoDuktapeRoot) {
    let tokens = tokenizeFiles({ filelist: commandPositional });
    Object.getOwnPropertyNames(tokens).sort().forEach((token) => { console.log(token); });
}

exports.tokenizeCommand = tokenizeCommand;
