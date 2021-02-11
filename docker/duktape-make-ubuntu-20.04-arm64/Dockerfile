FROM duktape-base-ubuntu-20.04-arm64:latest

COPY --chown=duktape:duktape run.sh .
RUN chmod 755 run.sh

# Result is written to stdout as a ZIP, which allows caller to extract
# result without uid issues (ZIP tolerates leading garbage).
ENTRYPOINT ["/work/run.sh"]
