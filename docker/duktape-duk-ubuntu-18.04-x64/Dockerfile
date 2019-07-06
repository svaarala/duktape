FROM duktape-base-ubuntu-18.04-x64:latest

COPY --chown=duktape:duktape run.sh .
RUN chmod 755 run.sh

# Dist result is written to stdout as a ZIP, which allows caller to extract
# result without uid issues (ZIP tolerates leading garbage).
CMD /work/run.sh
