
@.str.main = private unnamed_addr constant [10 x i8] c"main: %d\0A\00", align 1
@.str.coro = private unnamed_addr constant [10 x i8] c"coro: %d\0A\00", align 1

declare noalias ptr @malloc(i32 noundef)
declare void @free(ptr noundef)
declare void @llvm.trap()

declare token @llvm.coro.id.retcon.once(i32, i32, ptr, ptr, ptr, ptr)
declare ptr @llvm.coro.begin(token, ptr writeonly)
declare i1 @llvm.coro.suspend.retcon.i1(...)
declare i1 @llvm.coro.end(ptr, i1)
declare ptr @llvm.coro.prepare.retcon(ptr)

declare i32 @printf(i8*, ...)

declare void @_cont_prototype(ptr, i1 zeroext)


define {ptr, i32*} @negated(ptr %buffer, i32* %x.addr) presplitcoroutine {
entry:
  ; Begin the coroutine; the frame buffer is passed in from the caller.
  %id = call token @llvm.coro.id.retcon.once(i32 8, i32 8, ptr %buffer, ptr @_cont_prototype, ptr @malloc, ptr @free)
  %hdl = call ptr @llvm.coro.begin(token %id, ptr null)

  ; Create local variably `y` and store in it `-x`.
  %y.addr = alloca i32
  %x.val = load i32, i32* %x.addr
  %x.neg = sub nsw i32 0, %x.val
  store i32 %x.neg, i32* %y.addr

  ; Transfer control back to the caller, passing the address of `y`.
  %0 = call i1 (...) @llvm.coro.suspend.retcon.i1(ptr %y.addr)

  ; Put the value from `y` back into `x` (negating it again).
  %y.val = load i32, i32* %y.addr
  %y.neg = sub nsw i32 0, %y.val
  store i32 %y.neg, i32* %x.addr

  ; Done.
  call i1 @llvm.coro.end(ptr %hdl, i1 false)
  unreachable
}

define i32 @do_test(i32 %x.val.initial) {
entry:
  ; Create a local variable `x` with the given value.
  %x.addr = alloca i32
  store i32 %x.val.initial, i32* %x.addr

  ; %tmp1 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 1)

  ; Alloc frame buffer, and call the coroutine
  %frame_buffer = alloca [8 x i8], align 8
  %prepare = call ptr @llvm.coro.prepare.retcon(ptr @negated)
  %result0 = call { ptr, i32* } %prepare(ptr %frame_buffer, ptr %x.addr)

  ; Look at the data returned from the coroutine (continuation and address of `y`).
  %cont0 = extractvalue { ptr, i32* } %result0, 0
  %y.addr = extractvalue { ptr, i32* } %result0, 1

  ; Increment the `y` value.
  %y.val = load i32, i32* %y.addr
  %y.inc = add nsw i32 %y.val, 1
  store i32 %y.inc, i32* %y.addr

  ; Resume the coroutine.
  call void %cont0(ptr %frame_buffer, i1 false)

  ; Return the last value in our variable `x`.
  %x.val = load i32, i32* %x.addr
  ret i32 %x.val
}
