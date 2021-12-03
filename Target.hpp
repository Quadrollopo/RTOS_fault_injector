#include <string>

using namespace std;

class Target{
private:

private:
    string name;
    long address;
    int size;
public:
    Target(string name, long address, int size) : name(name), address(address), size(size) {}

    string getName() const {
        return name;
    }

    long getAddress() const {
        return address;
    }

    int getSize() const {
        return size;
    }

};