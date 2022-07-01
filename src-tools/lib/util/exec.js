'use strict';

const { exec, execStdoutUtf8, asyncExecStdoutUtf8 } = require('../extbindings/exec');
exports.exec = exec;
exports.execStdoutUtf8 = execStdoutUtf8;
exports.asyncExecStdoutUtf8 = asyncExecStdoutUtf8;
