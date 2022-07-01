'use strict';

const { readFileYaml } = require('../util/fs');

function releasesYamlToRst(fn) {
    var doc = readFileYaml(fn);
    var res = [];
    var convertGhLinks = true;

    res.push('================');
    res.push('Duktape releases');
    res.push('================');

    function rstEscape(x) {
        x = x.replace(/\\/g, () => { return '\\\\'; });
        x = x.replace(/\*/g, () => { return '\\*'; });
        return x;
    }

    function convertGithubIssueLinks(x) {
        return x.replace(/GH-(\d+)/g, function (c, issueNumber) {
            var ghLink = 'https://github.com/svaarala/duktape/issues/' + issueNumber; // Works for both pulls and issues.
            return '`' + c + ' <' + ghLink + '>`_';
        });
    }

    function emitWrappedRstBullet(x) {
        x = x.trim().replace(/_/g, '\\_');
        if (convertGhLinks) {
            x = convertGithubIssueLinks(x);
        }
        var parts = x.split(/ (?!<)/g); // Avoid splitting inline links.
        var lines = ['*'];
        var wrapLimit = 80;
        while (parts.length > 0) {
            let currLine = lines[lines.length - 1];
            if (currLine.length > 0 && currLine.length + parts[0].length >= wrapLimit) {
                lines.push(' ');
            }
            lines[lines.length - 1] += ' ' + parts[0];
            void parts.shift();
        }
        lines = lines.map((line) => line.replace(/\s+$/, ''));
        lines.forEach((line) => { res.push(line) });
    }

    function emitChange(change) {
        let changeDesc = typeof change === 'string' ? change : change.description;
        res.push('');
        emitWrappedRstBullet(rstEscape(changeDesc));
    }

    function emitReleases(released) {
        doc.duktape_releases.forEach((rel) => {
            let isReleased = !!rel.release_date;
            if (released !== isReleased) {
                return;
            }
            let relTitle = rel.version + ' (' + (rel.release_date || 'XXXX-XX-XX') + ')';
            res.push('');
            res.push(relTitle);
            res.push(('-').repeat(relTitle.length));
            //res.push('');
            //res.push('* DESCRIPTION: ' + rel.description + (rel.maintained === false ? ' (no longer maintained)' : ''));
            if (rel.changes_sections) {
                rel.changes_sections.forEach((section) => {
                    let sectionTitle = section.title + ':';
                    res.push('');
                    res.push(sectionTitle);
                    section.changes.forEach(emitChange);
                });
            } else {
                rel.changes.forEach(emitChange);
            }
        });
    }

    res.push('');
    res.push('Released');
    res.push('========');
    emitReleases(true);
    res.push('');
    res.push('Planned');
    res.push('=======');
    emitReleases(false);

    return res.join('\n') + '\n';
}
exports.releasesYamlToRst = releasesYamlToRst;
