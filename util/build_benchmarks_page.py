#!/usr/bin/env python2

import os
import sys
import re
import json

def main():
    duktape_repo = '/home/duktape/duktape'
    duktape_testrunner_repo = '/home/duktape/duktape-testrunner'
    duktape_testclient_config = '/home/duktape/duktape-testclient-config.yaml'
    benchmarks_template = '/home/duktape/duktape/website/benchmarks.html'
    merge_count = 1000

    # Get the hashes we're interested in, in increasing merge order.

#    os.system('cd %s && git pull --rebase' % duktape_repo)
    os.system('cd %s && git log -n %d --merges --oneline --decorate=no --pretty=format:%%H > /tmp/tmp-hashes.txt' % (duktape_repo, merge_count))
    hashes = []
    with open('/tmp/tmp-hashes.txt', 'rb') as f:
        for line in f:
            line = line.strip()
            if line != '':
                hashes.append(line)
    hashes.reverse()
    print('%d hashes found' % len(hashes))

    # Get any release tags matching the hashes for annotations.
    re_release_tag = re.compile('^v\d+\.\d+\.\d+$')
    annotations = []
    for x,h in enumerate(hashes):
        os.system('git tag -l --points-at %s > /tmp/tmp-taglog.txt' % h)
        with open('/tmp/tmp-taglog.txt', 'rb') as f:
            for line in f:
                line = line.strip()
                m = re_release_tag.match(line)
                if m is None:
                    continue
                annotations.append({ 'x': x, 'tag': line })
    print(json.dumps(annotations, indent=4))

    req = { 'repo_full': 'svaarala/duktape', 'sha_list': hashes }
    with open('/tmp/tmp-request.json', 'wb') as f:
        f.write(json.dumps(req))

    os.system('cd %s && cd client-simple-node && nodejs client.js --request-uri /query-commit-simple --config %s --request-file /tmp/tmp-request.json --output-file /tmp/tmp-result.json' % (duktape_testrunner_repo, duktape_testclient_config))

    with open(benchmarks_template, 'rb') as f:
        page = f.read()
    with open('/tmp/tmp-result.json', 'rb') as f:
        data = json.loads(f.read())

    doc = {
        'commit_simples': data,
        'annotations': annotations
    }
    doc_formatted = json.dumps(doc, indent=4)
    page = page.replace('<!-- @DATA@ -->', \
                        'var rawGraphData = \n' + doc_formatted + '\n')
    with open('/tmp/benchmarks.html', 'wb') as f:
        f.write(page)

if __name__ == '__main__':
    main()
