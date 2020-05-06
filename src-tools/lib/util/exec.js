'use strict';

const { exec, execStdoutUtf8 } = require('../extbindings/exec');
exports.exec = exec;
exports.execStdoutUtf8 = execStdoutUtf8;
