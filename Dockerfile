FROM alpine
LABEL stage="build"
COPY . /usr/src/ripple
RUN apk update && (rm /usr/src/ripple/main || true) && (rm -r /usr/src/ripple/build || true)
RUN apk add clang make musl-dev gdb sqlite-dev uriparser-dev openssl-dev
WORKDIR /usr/src/ripple
RUN make main

FROM alpine
LABEL stage="final"
RUN apk update && apk add vim sqlite-dev uriparser-dev openssl-dev
COPY --from=0 /usr/src/ripple/main /bin/main
ENTRYPOINT [ "/bin/main" ]
CMD [ "config/channel_list.txt" ]