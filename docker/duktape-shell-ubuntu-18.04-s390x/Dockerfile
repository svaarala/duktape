FROM duktape-base-ubuntu-18.04-s390x:latest

COPY --chown=duktape:duktape run.sh .
RUN chmod 755 run.sh

CMD /work/run.sh
