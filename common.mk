CXX=c++
CXXFLAGS=-std=c++17 -c -O2
CPPFLAGS=
LDFLAGS=-std=c++17 -s

AR=ar
ARFLAGS=rcs

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<
