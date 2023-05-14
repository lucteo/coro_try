
@.str.main = private unnamed_addr constant [10 x i8] c"main: %d\0A\00", align 1
@.str.coro = private unnamed_addr constant [10 x i8] c"coro: %d\0A\00", align 1

declare noalias ptr @malloc(i32 noundef)
declare void @free(ptr noundef)
declare void @llvm.trap()

declare i1 @llvm.coro.alloc(token)
declare void @llvm.coro.destroy(i8*)
declare i8* @llvm.coro.free(token, i8*)
declare i8* @llvm.coro.begin(token, i8*)
declare i1 @llvm.coro.end(i8*, i1)
declare token @llvm.coro.id(i32, i8*, i8*, i8*)
declare i8* @llvm.coro.promise(i8*, i32, i1)
declare void @llvm.coro.resume(i8*)
declare i32 @llvm.coro.size.i32()
declare i8 @llvm.coro.suspend(token, i1)

declare i32 @printf(i8*, ...)


define i8* @negated(i32* %x.addr) presplitcoroutine {
entry:
  ; Alloca the promise object (the value `y`).
  %promise = alloca i32
  %pv = bitcast i32* %promise to i8*

  ; Do we need dynamic heap allocation?
  %id = call token @llvm.coro.id(i32 0, i8* %pv, i8* null, i8* null)
  %need.dyn.alloc = call i1 @llvm.coro.alloc(token %id)
  ; %tmp1 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 1)
  br i1 %need.dyn.alloc, label %dyn.alloc, label %coro.begin
dyn.alloc:
  ; If yes, perform the memory allocation.
  ; %tmp2 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 2)
  %size = call i32 @llvm.coro.size.i32()
  %alloc = call i8* @malloc(i32 %size)
  br label %coro.begin
coro.begin:
  ; Begin the coroutine on the given memory allocation.
  %phi = phi i8* [ null, %entry ], [ %alloc, %dyn.alloc ]
  ; %tmp3 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 3)
  %hdl = call noalias i8* @llvm.coro.begin(token %id, i8* %phi)
  br label %body.start

body.start:
  ; %tmp4 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 4)
  ; Store in promise the value of `-x`
  %x.val = load i32, i32* %x.addr
  %x.neg = sub nsw i32 0, %x.val
  store i32 %x.neg, i32* %promise

  ; Transfer control back to the caller.
  %0 = call i8 @llvm.coro.suspend(token none, i1 false)
  switch i8 %0, label %suspend [i8 0, label %body.continue
                                i8 1, label %cleanup]

body.continue:
  ; %tmp5 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 5)
  ; Put the value from the promise into `x`.
  %y.val = load i32, i32* %promise
  %y.neg = sub nsw i32 0, %y.val
  store i32 %y.neg, i32* %x.addr
  br label %cleanup

cleanup:
  ; %tmp6 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 6)
  ; Cleanup the coroutine frame.
  %mem = call i8* @llvm.coro.free(token %id, i8* %hdl)
  call void @free(i8* %mem)
  br label %suspend

suspend:
  ; %tmp7 = call i32 (ptr, ...) @printf(ptr @.str.coro, i32 7)
  ; The coroutine is done.
  %unused = call i1 @llvm.coro.end(i8* %hdl, i1 false)
  ret i8* %hdl
}

define i32 @do_test(i32 %x.val.initial) {
entry:
  ; Create a local variable `x` with the given value.
  %x.addr = alloca i32
  store i32 %x.val.initial, i32* %x.addr

  ; %tmp1 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 1)

  ; Call the coroutine and get the promise object.
  %hdl = call i8* @negated(i32* %x.addr)
  %promise.addr.raw = call i8* @llvm.coro.promise(i8* %hdl, i32 4, i1 false)
  %promise.addr = bitcast i8* %promise.addr.raw to i32*
  ; %tmp2 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 2)

  ; Increment the value in the promise.
  %y.val = load i32, i32* %promise.addr
  %y.inc = add nsw i32 %y.val, 1
  store i32 %y.inc, i32* %promise.addr
  ; %tmp3 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 3)

  ; Resume the coroutine.
  call void @llvm.coro.resume(i8* %hdl)
  ; %tmp4 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 4)

  ; Done. Destroy the coroutine.
  ; call void @llvm.coro.destroy(i8* %hdl)
  ; %tmp5 = call i32 (ptr, ...) @printf(ptr @.str.main, i32 5)

  ; Return the last value in our variable `x`.
  %x.val = load i32, i32* %x.addr
  ret i32 %x.val
}
