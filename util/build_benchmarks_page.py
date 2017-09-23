#!/usr/bin/env python2

import os
import sys
import re
import json

def main():
    # Adapt manually.

    duk = '/usr/local/bin/duk'
    lzstring = '/home/duktape/duktape/lz-string/libs/lz-string.js'
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
        os.system('cd %s && git tag -l --points-at %s > /tmp/tmp-taglog.txt' % (duktape_repo, h))
        with open('/tmp/tmp-taglog.txt', 'rb') as f:
            for line in f:
                line = line.strip()
                m = re_release_tag.match(line)
                if m is None:
                    continue
                annotations.append({ 'x': x, 'tag': line })
    print(json.dumps(annotations, indent=4))

    # Get test data for hashed, and pack it into a JSON object embedded
    # into the page.

    req = { 'repo_full': 'svaarala/duktape', 'sha_list': hashes }
    with open('/tmp/tmp-request.json', 'wb') as f:
        f.write(json.dumps(req))

    os.system('cd %s && cd client-simple-node && nodejs client.js --request-uri /query-commit-simple --config %s --request-file /tmp/tmp-request.json --output-file /tmp/tmp-result.json' % (duktape_testrunner_repo, duktape_testclient_config))

    with open('/tmp/tmp-result.json', 'rb') as f:
        data = json.loads(f.read())

        for commit in data:
            for run in commit.get('runs', []):
                # Censor some fields which take a lot of space
                if run.has_key('output_uri'):
                    del run['output_uri']
                if run.has_key('result') and run['result'].has_key('traceback'):
                    del run['result']['traceback']

    doc = {
        'commit_simples': data,
        'annotations': annotations
    }
    with open('/tmp/tmp-graphdata.json', 'wb') as f:
        f.write(json.dumps(doc))

    # There's a lot of JSON data so use http://pieroxy.net/blog/pages/lz-string/index.html
    # to compress it.  'duk' executable can be used to compress data.

    with open('/tmp/tmp-script.js', 'wb') as f:
        f.write('''
var input = new TextDecoder().decode(readFile('/tmp/tmp-graphdata.json'));
var compressed = LZString.compressToBase64(input);
writeFile('/tmp/tmp-graphdata-compressed.txt', compressed);
''')
    os.system('%s %s /tmp/tmp-script.js' % (duk, lzstring))
    with open('/tmp/tmp-graphdata-compressed.txt', 'rb') as f:
        graphdata = f.read()

    # Embed the compressed data into the benchmarks.html template.

    with open(benchmarks_template, 'rb') as f:
        page = f.read()

    page = page.replace('<!-- @DATA@ -->', \
                        'var rawGraphDataCompressed = "' + graphdata + '";')

    with open('/tmp/benchmarks.html', 'wb') as f:
        f.write(page)

    # Done!
    print('done')

if __name__ == '__main__':
    main()
