#include <string>
#include <utility>

using namespace std;

class Target{
private:

private:
    string name;
    long address;
    int size;
    bool pointer;
public:
    Target(string name, long address, int size, bool pointer=false) : name(std::move(name)), address(address), size(size), pointer(pointer){};

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