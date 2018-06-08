// UDP Client Server -- allow for any type of RAII deleter
// Copyright (c) 2018  Made to Order Software Corp.  All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#pragma once

// snapwebsites lib
//
#include <snapwebsites/not_used.h>

// C++ lib
//
#include <memory>



namespace snap
{


/** \brief A templated used to allow unique_ptr<>() of nearly any type.
 *
 * This template is used with nearly any type representing a resource
 * that you would like to automatically delete (RAII) by using a
 * unique_ptr<>() definition.
 *
 * Note that it only works with resources which type is not a pointer.
 * For example, a file descriptor which uses an `int` works with this
 * class.
 *
 * For resources that have a pointer, use the raii_pointer_deleter instead.
 *
 * For example, a file descriptor can use this template class as follow:
 *
 * \code
 * 	typedef std::unique_ptr<int, snap::raii_generic_deleter<int, -1, decltype(&::close), &::close>> raii_fd_t;
 *
 * 	raii_fd_t fd(open("/tmp/test.tmp", O_RDWR));
 * \endcode
 *
 * \note
 * This template class is based on code found in this answer on Stackoverflow:
 * https://stackoverflow.com/questions/15756960/using-unique-ptr-to-control-a-file-descriptor#48575395
 *
 * \param[in] T  The type being managed such as 'int'.
 * \param[in] null_value  The value representing "nullptr" for that type. For
 *                        a file descriptor this is -1.
 * \param[in] D  The type of the deleter function.
 * \param[in] deleter  The deleter function (i.e. 'deleter(p)' gets rid of `T p`).
 */
template<class T, T null_value, class D, D deleter>
class raii_generic_deleter
{
public:
    /** \brief The pointer class.
     *
     * This class must be called "pointer" because the unique_ptr<>()
     * template definition checks the deleter using the following
     * code:
     *
     * \code
	 * template<typename _Up>
	 *     static typename _Up::pointer __test(typename _Up::pointer*);
     * [...]
	 * typedef decltype(__test<_Del>(0)) type;
     * \endcode
     *
     * As we can see it looks for a class named pointer. If it exists,
     * then it defines the type using that pointer class pointer operator
     * (i.e. the `\<type> operator *`) instead of using an actual pointer
     * (i.e. it uses `deleter::pointer` instead of the default `T *`.)
     */
    class pointer
    {
    private:
        /** \brief The pointer being manage.
         *
         * This parameter is the resource pointer being managed.
         */
        T f_pointer;

    public:
        /** \brief Create a pointer with a value of type T.
         *
         * This constructor creates an explicit instance of a pointer with
         * a type T object.
         *
         * \param[in] p  The pointer being managed by this pointer class.
         */
        pointer(T p)
            : f_pointer(p)
        {
        }

        /** \brief Allow assigning the nullptr to this type.
         *
         * This constructor allows for the nullptr value to be assigned to
         * this type. This is a test run at compile time. If not present
         * the resulting std::unique_ptr<>() would not compile.
         *
         * \param[in] p  The null pointer.
         */
        pointer(std::nullptr_t p = nullptr)
            : f_pointer(null_value)
        {
            NOTUSED(p);
        }

//        /** \brief Assign the nullptr to this pointer.
//         *
//         * This function is used to assign `nullptr` to this pointer.
//         *
//         * \warning
//         * It is assumed that this is called only when the pointer
//         * was released by a call to the `deleter` function.
//         *
//         * \param[in] h  The `nullptr` being assigned.
//         *
//         * \return A reference to this pointer.
//         */
//        pointer & operator = (std::nullptr_t h)
//        {
//            NOTUSED(h);
//
//            f_pointer = null_value;
//
//            return *this;
//        }

        /** \brief Convert pointer to bool.
         *
         * This operator allows for converting the pointer to a bool
         * which is often done in an `if(ptr)` or equivalent statement.
         *
         * \return true if the pointer is `null_value`.
         */
        explicit operator bool () const
        {
            return f_pointer != null_value;
        }

        /** \brief Compare two pointers against each other for equality.
         *
         * This function compares the 'this' and \p rhs pointers against
         * each other. If equal then it returns true.
         *
         * \param[in] rhs  The right hand side pointer being compared against
         *                 `this` pointer.
         *
         * \return true if both pointers are equal.
         */
        bool operator == (pointer const rhs) const
        //friend bool operator == (pointer lhs, pointer rhs) //const
        {
            return f_pointer == rhs.f_pointer;
        }

        /** \brief Compare two pointers against each other for inequality.
         *
         * This function compares the `this` and \p rhs pointers against
         * each other. If not equal then it returns true.
         *
         * \param[in] rhs  The right hand side pointer being compared against
         *                 `this` pointer.
         *
         * \return true if both pointers are not equal.
         */
        bool operator != (pointer const rhs) const
        //friend bool operator != (pointer lhs, pointer rhs)
        {
            return f_pointer != rhs.f_pointer;
        }

//        /** \brief Compare this pointer against nullptr for equality.
//         *
//         * This function is used whenever this pointer gets compared
//         * against the special value `nullptr`.
//         *
//         * \param[in] rhs  The nullptr.
//         *
//         * \return true when this pointer is the `nullptr`.
//         */
//        bool operator == (std::nullptr_t rhs) const
//        {
//            NOTUSED(rhs);
//            return f_pointer == null_value;
//        }
//
//        /** \brief Compare this pointer against nullptr for inequality.
//         *
//         * This function is used whenever this pointer gets compared
//         * against the special value `nullptr` for inquality.
//         *
//         * \param[in] rhs  The nullptr.
//         *
//         * \return true when this pointer is not the `nullptr`.
//         */
//        bool operator != (std::nullptr_t rhs) const
//        {
//            NOTUSED(rhs);
//            return f_pointer != null_value;
//        }

        /** \brief Retrieve the pointer.
         *
         * This function is used to retrieve the pointer.
         *
         * \return A copy of the resource pointer.
         */
        T & operator * ()
        {
            return f_pointer;
        }
    };

    /** \brief The function called to delete/release this pointer.
     *
     * This function gets called whenever the unique_ptr<>() or
     * shared_ptr<>() decides to delete the pointer.
     *
     * \note
     * The function gets called only if p does not represent nullptr
     * and the handle gets cleaned in the unique_ptr<>() as required.
     *
     * \param[in] h  The pointer to delete.
     */
    void operator () (pointer p)
    {
        deleter(*p);
    }
};


/** \brief A templated used to easily delete a unique_ptr<>() resource.
 *
 * This template is used with any type if resources represented by a pointer.
 * It automatically deletes (RAII) the resource one the unique_ptr<>()
 * goes out of scope.
 *
 * Note that it only works with resources which type is a pointer.
 * For example, a FILE object which uses a `FILE *` works with this
 * class.
 *
 * For resources that do not have a pointer, use the raii_generic_deleter
 * instead.
 *
 * For example, a FILE object can use this template class as follow:
 *
 * \code
 *      typedef std::unique_ptr<FILE, raii_pointer_deleter<FILE, decltype(&::fclose), &::fclose>> raii_file;
 *
 *      raii_file f(fopen("/tmp/test.tmp", "rw")); // f is automatically closed on a return or an interrupt
 * \endcode
 *
 * \param[in] T  The type being managed such as 'int'.
 * \param[in] D  The type of the deleter function.
 * \param[in] deleter  The deleter function (i.e. 'deleter(p)' gets rid of `T * p`).
 */
template<class T, class D, D deleter>
class raii_pointer_deleter
{
public:
    /** \brief The function called to delete/release this type of pointer.
     *
     * This function gets called whenever the unique_ptr<>() or
     * shared_ptr<>() decides to delete the pointer.
     *
     * \note
     * The function gets called only if p does not represent nullptr
     * and the pointer gets cleaned in the unique_ptr<>() as required.
     *
     * \param[in] h  The pointer to delete.
     */
    void operator () (T * p)
    {
        deleter(p);
    }
};


} // namespace snap
// vim: ts=4 sw=4 et
