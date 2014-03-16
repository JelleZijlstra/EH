#include "std_lib_includes.hpp"

#ifndef EH_BYTEARRAY_H_
#define EH_BYTEARRAY_H_
/*
 * ByteArray class
 */

EH_CLASS(ByteArray) {
public:
    struct eh_byte_array {
        uint8_t *content;
        size_t size;
        eh_byte_array(size_t size_) : content(new uint8_t[size_]()), size(size_) {}

        void resize(size_t new_size) {
            uint8_t *old_content = content;
            uint8_t *new_content = new uint8_t[size]();
            memcpy(new_content, old_content, std::min(size, new_size));
            content = new_content;
            delete[] old_content;
        }
        bool has(int index) {
            return index >= 0 && ((size_t) index) < size;
        }
        ~eh_byte_array() {
            delete[] content;
        }
    };
    typedef eh_byte_array *type;
    const type value;

    static ehval_p make(size_t size) {
        return static_cast<ehval_t *>(new ByteArray(size));
    }

    virtual bool belongs_in_gc() const {
        return false;
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
        std::cout << "@bytearray <";
        for(size_t i = 0; i < value->size; i++) {
            std::cout << int(value->content[i]) << ",";
        }
        std::cout << ">" << std::endl;
    }

    virtual std::string decompile(int level) const override {
        return "<bytearray>";
    }
private:
    ByteArray(uint8_t f) : value(new eh_byte_array(f)) {}
};

EH_METHOD(ByteArray, operator_colon);
EH_METHOD(ByteArray, operator_arrow);
EH_METHOD(ByteArray, operator_arrow_equals);
EH_METHOD(ByteArray, length);
EH_METHOD(ByteArray, resize);
EH_METHOD(ByteArray, getInteger);
EH_METHOD(ByteArray, setInteger);

EH_INITIALIZER(ByteArray);

#endif /* EH_BYTEARRAY_H_ */
