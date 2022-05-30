//
// Created by shay on 2022/5/30.
//

#include <iostream>
#include <vector>

namespace my_stack
{
	class Obj
	{
	public:
		Obj()
		{
			std::cout << "Obj()\n";
		};

		~Obj()
		{
			std::cout << "~Obj()\n";
		}
	};

	void foo(int n)
	{
		Obj obj;
		if (n == 42)
			throw "life, the universe and everything";
	}
}

namespace my_raii
{
	// 工厂模式 返回一个 shape 对象，实际是 shape 的某个子类 circle、triangle、rectangle
	enum class shape_type
	{
		circle,
		triangle,
		rectangle
	};

	class shape
	{
	public:
		shape() = default;

		virtual ~shape() = default;
	};

	class circle : public shape
	{
	public:
		circle() = default;

		~circle() override = default;
	};

	class rectangle : public shape
	{
	public:
		rectangle() = default;

		~rectangle() override = default;
	};

	class triangle : public shape
	{
	public:
		triangle() = default;

		~triangle() override = default;
	};

	// 如果返回值是 shape 编译器不会报错，对象切片（object slicing）对象复制相关的语义错误
	shape* create_shape(shape_type type)
	{
		// new 时先分配内存，分配失败抛出异常（bad_alloc），成功后在这个结果指针上构造对象。
		switch (type)
		{
		case shape_type::circle:
			return new circle();
		case shape_type::triangle:
			return new triangle();
		case shape_type::rectangle:
			return new rectangle();
		}
	}

	class shape_wrapper
	{
	public:
		explicit shape_wrapper(shape* ptr = nullptr) : ptr_(ptr)
		{
		}

		~shape_wrapper()
		{
			// delete 空指针是合法操作
			// delete 在指针不为空时调用析构函数，释放分配内存。

			// 这里做必须的清理工作：关闭文件、释放同步锁、释放其他重要系统资源
			// std::lock_guard<std::mutex> 优先于 mutex.lock() mutex.unlock()
			delete ptr_;
		}

		[[nodiscard]] shape* get() const
		{
			return ptr_;
		};
	private:
		shape* ptr_;
	};

	void foo()
	{
		shape_wrapper ptr_wrapper(create_shape(shape_type::circle));
	}
}

int main()
{
	// 堆（heap）：动态分配内存堆区域。需要手动释放内存，否则内存泄漏。
	// new 和 delete 操作自由存储区（free store）-- 堆堆子集
	// malloc 和 free 操作堆区
	// new 和 delete 底层通常使用 malloc 和 free 实现

	// 动态内存不确定性：分配耗时、分配失败怎么处理

	// 内存管理器
	// 1 让内存管理器分配某个大小的内存快 当前有多少未分配内存，内存不足向 OS 申请新内存，内存充足，做簿记工作标记内存已用，分配内存。
	// 2 让内存管理器释放只欠分配的内存块 连续未使用内存块合并成一块。
	[[maybe_unused]] auto ptr = new std::vector<int>();


	// 栈（stack）：函数调用过程中产生本地变量和调用数据堆区域 -- 先进后出
	// 栈从高地址增长到低地址，新函数进入，做保存工作，调整栈指针，分配本地变量所需空间，执行函数，返回调用者未执行代码初继续执行
	// 栈分配回收移动栈指针即可，先进后出不存在内存碎片
	try
	{
		my_stack::foo(41);
		my_stack::foo(42);
	}
	catch (const char* s)
	{
		std::cout << s << std::endl;
	}

	// RAII（Resource Acquisition Is Initialization）：通过栈和析构函数对所有资源进行管理。
	// 对象很大，对象在编译期不能确定大小，对象的返回值不应该使用 -- 对象不应该存在栈上


	return 0;
}