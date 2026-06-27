 /******* CustomImGuiAllocator.h ***************************************************/ /**
 *
 * @file CustomImGuiAllocator.h
 *
 * Forward declaration of the class CustomImGuiAllocator
 *
 * @version 1
 * @author SpookyProDH-Coder
 * @date 25/06/2026
 ***************************************************************************/

#ifndef _H_CUSTOMIMGUIALLOCATOR
#define _H_CUSTOMIMGUIALLOCATOR

#include <bx/allocator.h>

/**
 * @brief Compatible bgfx allocator for ImGui::CreateContext
 */
class CustomImGuiAllocator : public bx::AllocatorI
{
    public:
        virtual ~CustomImGuiAllocator() {}
        virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _filePath, uint32_t _line) override 
        {
            (void)_align; (void)_filePath; (void)_line;
            if (_size == 0) 
            {
                if (_ptr != nullptr) 
                    std::free(_ptr);
                return nullptr;
            }
            else if (_ptr == nullptr)
                return std::malloc(_size);
            else
                return std::realloc(_ptr, _size);
        }
};

#endif