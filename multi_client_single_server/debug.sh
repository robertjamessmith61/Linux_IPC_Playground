if [ ! -d build ]; then mkdir build; fi && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && cd ..
