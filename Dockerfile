# Build container
FROM viros/is-sdk:1

ENV LD_LIBRARY_PATH /is-sdk/lib
ADD . /is-sdk/src/picom
RUN /is-sdk/bin/install_cmake /is-sdk/src/picom          \
 && mkdir /is-sdk/bin/deploy                             \
 && libs=`find /is-sdk/bin/*.bin -exec ldd {} \;         \
    | cut -d '(' -f 1 | cut -d '>' -f 2 | sort | uniq`   \
 && for lib in $libs;                                    \
    do                                                   \  
      dir="/is-sdk/bin/deploy`dirname $lib`";            \
      mkdir -v -p  $dir;                                 \
      cp --verbose $lib $dir;                            \
    done

# Deployment container
FROM scratch
ENV LD_LIBRARY_PATH /is-sdk/lib
EXPOSE 8080/tcp
COPY --from=0 /is-sdk/bin/deploy/ /
COPY --from=0 /is-sdk/bin/*.bin /
