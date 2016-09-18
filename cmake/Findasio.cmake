# Locates Asio networking library
#   http://think-async.com/Asio
#

find_path(
  asio_INCLUDE_DIR
  NAMES
    asio.hpp
  PATHS
    ${ASIO_DIR}
    /usr/local/include
    /usr/include
)

set(asio_FOUND "NO")

if(asio_INCLUDE_DIR)
  set(asio_FOUND "YES")
endif()

