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
    string subName;
public:
    Target(string name, long address, int size, bool pointer=false, string subName="") : name(std::move(name)), address(address), size(size), pointer(pointer), subName(std::move(subName)){};

    string getName() const {
        return name;
    }

    long getAddress() const {
        return address;
    }

    int getSize() const {
        return size;
    }

    bool isPointer() const{
        return pointer;
    }

    void setAddress(long ad) {
        this->address = ad;
    }

    void setSubName(const string &sn) {
        Target::subName = sn;
    }

    const string &getSubName() const {
        return subName;
    }

    void setPointer(bool p) {
        Target::pointer = p;
    }

};