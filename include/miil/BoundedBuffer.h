#ifndef BOUNDED_BUFFER_H
#define BOUNDED_BUFFER_H

#include <mutex>
#include <vector>
#include <iterator>
#include <condition_variable>
#include <chrono>

template <typename T>
class BoundedBuffer {
    std::mutex lock;
    std::vector<T> buffer;
    size_t buffer_capacity;
    size_t free_space;
    bool buffer_full;
    std::condition_variable cv_data_added;
public:
    /*!
     * \brief Allocates buffer space
     *
     * calls reserve (without try/catch) to allocate buffer
     *
     * \param capacity the size that should be allocated for the buffer
     */
    BoundedBuffer(size_t capacity) :
        buffer_capacity(capacity),
        free_space(capacity)
    {
        buffer.reserve(buffer_capacity);
    }

    /*!
     * \brief Copies container into buffer and clears the container
     *
     * Waits if buffer is locked.
     *
     * \param begin random access iterator
     * \param end random access iterator
     */
    template <typename Iterator>
    void insert(Iterator begin, Iterator end) {
        lock.lock();
        if (!buffer_full) {
            size_t no_entries = distance(begin, end);
            // Insert up to the first n = free_space entries
            if (no_entries > free_space) {
                end = begin + free_space;
                no_entries = free_space;
                buffer_full = true;
            }
            buffer.insert(buffer.end(), begin, end);
            free_space -= no_entries;
            cv_data_added.notify_all();
        }
        lock.unlock();
    }

    /*!
     * \brief Tries to copy container into buffer and clear the container
     *
     * Bails if buffer is locked.
     *
     * \param begin random access iterator
     * \param end random access iterator
     */
    template <typename Iterator>
    void try_insert(Iterator begin, Iterator end) {
        if (lock.try_lock()) {
            if (!buffer_full) {
                size_t no_entries = distance(begin, end);
                // Insert up to the first n = free_space entries
                if (no_entries > free_space) {
                    end = begin + free_space;
                    no_entries = free_space;
                    buffer_full = true;
                }
                buffer.insert(buffer.end(), begin, end);
                free_space -= no_entries;
                cv_data_added.notify_all();
            }
            lock.unlock();
        }
    }

    /*!
     * \brief Tries to copy container into buffer and clear the container
     *
     * Bails if buffer is locked.
     *
     * \param container a std library container with random access iterators
     */
    template <typename Container>
    void try_insert(Container & insert_buffer) {
        if (lock.try_lock()) {
            if (!buffer_full) {
                size_t no_entries = insert_buffer.size();
                if (no_entries > free_space) {
                    no_entries = free_space;
                    buffer_full = true;
                }
                buffer.insert(buffer.end(),
                              insert_buffer.begin(),
                              insert_buffer.begin() + no_entries);
                free_space -= no_entries;
            }
            lock.unlock();
            insert_buffer.clear();
            cv_data_added.notify_all();
        }
    }

    /*!
     * \brief Copies the contents of container into buffer the clears container
     *
     * Waits for a lock on the buffer
     *
     * \param container a std library container with random access iterators
     */
    template <typename Container>
    void insert(Container & insert_buffer) {
        lock.lock();
        if (!buffer_full) {
            size_t no_entries = insert_buffer.size();
            if (no_entries > free_space) {
                no_entries = free_space;
                buffer_full = true;
            }
            buffer.insert(buffer.end(),
                          insert_buffer.begin(),
                          insert_buffer.begin() + no_entries);
            free_space -= no_entries;
        }
        lock.unlock();
        insert_buffer.clear();
        cv_data_added.notify_all();
    }


    /*!
     * \brief Copies the buffer into a standard container
     *
     * Will wait for a lock on the buffer.
     *
     * \param container a std library container with insert() and end()
     */
    template <typename Container>
    void copy(Container & ret) {
        lock.lock();
        ret.insert(ret.end(), buffer.begin(), buffer.end());
        lock.unlock();
    }

    /*!
     * \brief Tries to copy the buffer into a standard container
     *
     * Will bail if the buffer is locked.
     *
     * \param container a std library container with insert() and end()
     */
    template <typename Container>
    void copy(Container & ret) {
        if (lock.try_lock()) {
            ret.insert(ret.end(), buffer.begin(), buffer.end());
            lock.unlock();
        }
    }

    /*!
     * \brief Returns if the buffer is full.
     */
    bool full() {
        lock.lock();
        bool full_val = buffer_full;
        lock.unlock();
        return(full_val);
    }

    /*!
     * \brief Returns if the buffer is empty
     */
    bool empty() {
        lock.lock();
        bool empty_val = (buffer.size() == 0) ? true:false;
        lock.unlock();
        return(empty_val);
    }

    /*!
     * \brief Clears the contents of the buffer
     *
     * Is a locking function.
     */
    void clear() {
        lock.lock();
        buffer.clear();
        free_space = buffer_capacity;
        buffer_full = false;
        lock.unlock();
    }

    /*!
     * \brief Tries to clear the contents of the buffer
     *
     * Skips the clear if the buffer is locked.
     */
    void try_clear() {
        if (lock.try_lock()) {
            buffer.clear();
            free_space = buffer_capacity;
            buffer_full = false;
            lock.unlock();
        }
    }

    /*!
     * \brief Copy contents of buffer into container and clear buffer
     *
     * Copies the contents of BoundedBuffer into the given standard container
     * and then clears the BoundedBuffer.
     *
     * \param container a std library container with insert() and end()
     */
    template <typename Container>
    void copy_and_clear(Container & container) {
        lock.lock();
        container.insert(container.end(), buffer.begin(), buffer.end());
        buffer.clear();
        free_space = buffer_capacity;
        buffer_full = false;
        lock.unlock();
    }

    /*!
     * \brief Waits for data to be put into the buffer, then copies and clears
     *
     * Waits for an insert or try insert function to be called, and then calls
     * copy and clear.  This still waits even if there is data in the buffer.
     * This could be modified later on.  If the function times out, nothing
     * happens to the container.
     *
     * \param container a std library container with insert() and end()
     * \param timeout_ms The number of milliseconds to wait before exiting
     *
     * \see copy_and_clear
     */
    template <typename Container>
    void wait_for_pull_all(Container & container, int timeout_ms) {
        std::mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);
        if (cv_data_added.wait_for(lck, std::chrono::milliseconds(timeout_ms))
            != std::cv_status::timeout)
        {
            copy_and_clear(container);
        }
    }

    /*!
     *  \brief Returns the capacity of the buffer
     */
    size_t capacity() {
        lock.lock();
        size_t capacity = buffer_capacity;
        lock.unlock();
        return(capacity);
    }
};

#endif // BOUNDED_BUFFER_H
