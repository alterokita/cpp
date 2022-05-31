//
// Created by shay on 2022/5/31.
//

#include <utility>        // for std::swap
#include <iostream>        // for std::cout


namespace smart
{
	// 这个指针传递后就不能通过这个指针访问这个对象了
	template<typename T>
	class smart_ptr
	{
	public:
		// 拷贝构造，调用 other 对 release 方法释放指针所有权，所有权到 this.ptr
		smart_ptr(const smart_ptr& other) noexcept
		{
			ptr_ = other.release();
		}

		// 赋值拷贝构造，构造临时对象调用 swap 交换指针所有权，临时对象自动销毁，指针所有权释放
		smart_ptr& operator=(const smart_ptr& rhs)
		{
			smart_ptr(rhs).swap(*this);
			return *this;
		}

		T* release()
		{
			T* ptr = ptr_;
			ptr_ = nullptr;
			return ptr;
		}

		void swap(smart_ptr& rhs)
		{
			std::swap(ptr_, rhs.ptr_);
		}

	private:
		T* ptr_;
	};
}

namespace smart2
{
	template<typename T>
	class smart_ptr
	{
	public:
		// 移动构造函数 -- 提供移动构造函数，但是没有手动提供拷贝构造函数，拷贝构造函数自动被禁用
		smart_ptr(const smart_ptr&& other) noexcept
		{
			ptr_ = other.release();
		}

		// 赋值函数是移动还是拷贝依赖于构造参数时用当是移动构造还是拷贝构造
		smart_ptr& operator=(smart_ptr rhs)
		{
			rhs.swap(*this);
			return *this;
		}

		T* release()
		{
			T* ptr = ptr_;
			ptr_ = nullptr;
			return ptr;
		}

		void swap(smart_ptr& rhs)
		{
			std::swap(ptr_, rhs.ptr_);
		}

	private:
		T* ptr_;
	};
}


// 共享计数
class shared_count
{
public:
	shared_count() noexcept: count_(1)
	{
	}

	// 增加计数
	void add_count() noexcept
	{
		++count_;
	}

	// 减少计数
	long reduce_count() noexcept
	{
		return --count_;
	}

	// 获取计数
	[[nodiscard]] long get_count() const noexcept
	{
		return count_;
	}

private:
	long count_;
};

// 类模版，将特定对象替换成 T （shape -> T）
template<typename T>
class smart_ptr
{
public:
	// 模版当各个实例之间不能访问私有成员，需要显示声明 friend 关系
	template<typename U>
	friend
	class smart_ptr;

	// 构造一个share_count
	explicit smart_ptr(T* ptr = nullptr) : ptr_(ptr)
	{
		if (ptr)
		{
			shared_count_ = new shared_count();
		}
	}

	// ptr_ 非空时 shared_count 也必然非空，引用数减一
	// 引用数为0时彻底删除对象和共享计数
	~smart_ptr()
	{
		std::cout << "~smart_ptr():" << this << "\n";
		if (ptr_ and !shared_count_->reduce_count())
		{
			delete ptr_;
			delete shared_count_;
		}
	}

	// 拷贝构造函数
	// 禁用拷贝构造函数，在编译期不会报错。但是运行期会有未定义行为，对同一内存释放两次 -- 程序崩溃
	// 拷贝时转移指针所有权 ？
	template<typename U>
	explicit smart_ptr(const smart_ptr<U>& other) noexcept
	{
		ptr_ = other.ptr_;
		if (ptr_)
		{
			other.shared_count_->add_count();
			shared_count_ = other.shared_count_;
		}
	}

	// 这个地方可以隐式将子类指针向基类指针转换
	// explicit 进制这种不明确转换
	template<typename U>
	explicit smart_ptr(smart_ptr<U>&& other) noexcept
	{
		ptr_ = other.ptr_;
		if (ptr_)
		{
			shared_count_ = other.shared_count_;
			other.ptr_ = nullptr;
		}
	}

	template<typename U>
	explicit smart_ptr(const smart_ptr<U>& other, T* ptr) noexcept
	{
		ptr_ = ptr;
		if (ptr_)
		{
			other.shared_count_->add_count();
			shared_count_ = other.shared_count_;
		}
	}

	// 拷贝构造函数
	smart_ptr& operator=(smart_ptr rhs) noexcept
	{
		rhs.swap(*this);
		return *this;
	}

	T* get() const noexcept
	{
		return ptr_;
	}

	[[nodiscard]] long use_count() const noexcept
	{
		if (ptr_)
		{
			return shared_count_->get_count();
		}
		else
		{
			return 0;
		}
	}

	void swap(smart_ptr& rhs) noexcept
	{
		std::swap(ptr_, rhs.ptr_);
		std::swap(shared_count_, rhs.shared_count_);
	}

	// 指针行为补全
	// * 解引用运算
	T& operator*() const noexcept
	{
		return *ptr_;
	}

	// -> 指向对象成员
	T* operator->() const noexcept
	{
		return ptr_;
	}

	// 运用在布尔表达式中
	explicit operator bool() const noexcept
	{
		return ptr_;
	}

private:
	T* ptr_;
	shared_count* shared_count_;
};

template<typename T>
void swap(smart_ptr<T>& lhs, smart_ptr<T>& rhs) noexcept
{
	lhs.swap(rhs);
}

// 实现 C++ 中的类型转换
template<typename T, typename U>
smart_ptr<T> static_pointer_cast(const smart_ptr<U>& other) noexcept
{
	T* ptr = static_cast<T*>(other.get());
	return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> dynamic_pointer_cast(const smart_ptr<U>& other) noexcept
{
	T* ptr = dynamic_cast<T*>(other.get());
	return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> reinterpret_pointer_cast(const smart_ptr<U>& other) noexcept
{
	T* ptr = reinterpret_cast<T*>(other.get());
	return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> const_pointer_cast(const smart_ptr<U>& other) noexcept
{
	T* ptr = const_cast<T*>(other.get());
	return smart_ptr<T>(other, ptr);
}