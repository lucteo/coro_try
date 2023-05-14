#include <coroutine>
#include <cassert>
#include <iostream>

/*
Target:
void negated(int* x) {
  int y = -x;
  yield &y;
  x = -y;
}
*/

namespace naive
{

  struct context
  {
    int *x;
    int y;
  };

  int *negated_start(char *buffer, int *x)
  {
    context *ctx = reinterpret_cast<context *>(buffer);
    ctx->x = x;
    int *y = &ctx->y;
    *y = -(*x);
    return y;
  }

  void negated_resume(char *buffer)
  {
    context *ctx = reinterpret_cast<context *>(buffer);
    int y = ctx->y;
    *ctx->x = -y;
  }

  int do_test(int x)
  {
    char buffer[256] = {0};
    int *x1 = negated_start(buffer, &x);
    (*x1)++;
    negated_resume(buffer);
    return x;
  }
}

namespace coro
{
  struct my_coro_task
  {
    struct promise_type
    {
      constexpr std::suspend_never initial_suspend() { return {}; }
      constexpr std::suspend_always final_suspend() noexcept { return {}; }
      std::suspend_always yield_value(int *y)
      {
        y_ = y;
        return {};
      }
      my_coro_task get_return_object() { return my_coro_task(std::coroutine_handle<promise_type>::from_promise(*this)); }
      void unhandled_exception() { std::abort(); }

      int *y_{nullptr};
    };

    my_coro_task(std::coroutine_handle<promise_type> h) : handle_(h) {}

    int* value() { return handle_.promise().y_; }

    void resume() { handle_(); }

  private:
    std::coroutine_handle<promise_type> handle_;
  };

  my_coro_task negated(int *x)
  {
    int y = -(*x);
    co_yield &y;
    *x = -y;
  }

  int do_test(int x)
  {
    auto coro_handle = negated(&x);
    int* y = coro_handle.value();
    (*y)++;
    coro_handle.resume();
    return x;
  }
}

#if defined(USE_NAIVE)
using naive::do_test;
#elif defined(USE_CXX_CORO)
using coro::do_test;
#else
extern "C" int do_test(int);
#endif

int main()
{
  auto x = do_test(5);
  std::cout << x << "\n";
  assert(x == 4);
}