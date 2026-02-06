FROM alpine
COPY . /usr/src/ripple
RUN apk update && rm /usr/src/ripple/main || true
RUN apk add clang make musl-dev gdb sqlite-dev uriparser-dev openssl-dev
WORKDIR /usr/src/ripple
RUN make main

FROM alpine
RUN apk update && apk add vim sqlite-dev uriparser-dev openssl-dev
COPY --from=0 /usr/src/ripple/main /bin/main
COPY --from=0 /usr/src/ripple/channel_list.txt channel_list.txt
CMD [ "/bin/main", "channel_list.txt"]