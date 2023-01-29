//
// Created by elf on 29/1/23.
//
#pragma once
#include <string>
#include <utility>

namespace toydb {
    template<typename T>
    struct Status {
        bool ok_;
        bool deleateable_ = true;
        union {
            std::string message_;
            T value_;
        };

        Status(): ok_(true), value_() {
        }

        explicit Status(std::string msg): ok_(false), message_(std::move(msg)) {
       }

        Status(Status const &status): ok_(status.ok_) {
            if (ok_) {
                value_ = std::move(status.value_);
            } else {
                message_ = std::move(status.message_);
            }
        }

        Status& operator=(Status const& other)
        {
            ok_ = other.ok_;
            if (ok_) {
                value_ = std::move(other.value_);
            } else {
                message_ = std::move(other.message_);
            }
            deleateable_ = other.deleateable_;

            return *this;
        }

        ~Status() {
            if (!ok_) {
                message_.~basic_string();
            } else if (deleateable_) {
                value_.~T();
            }
        }

        bool ok() {
            return ok_;
        }

    };
    template<typename T>
    Status<T> OkWithValue(T v) {
        Status<T> s;
        s.value_ = v;
        return s;
    }
}
