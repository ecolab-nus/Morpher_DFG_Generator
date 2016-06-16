; ModuleID = 'fourierf.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [52 x i8] c"Error in fft():  NumSamples=%u is not power of two\0A\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"RealIn\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"RealOut\00", align 1
@.str.3 = private unnamed_addr constant [8 x i8] c"ImagOut\00", align 1
@.str.4 = private unnamed_addr constant [35 x i8] c"Error in fft_float():  %s == NULL\0A\00", align 1

; Function Attrs: nounwind uwtable
define void @fft_float(i32 %NumSamples, i32 %InverseTransform, float* %RealIn, float* %ImagIn, float* %RealOut, float* %ImagOut) #0 {
entry:
  %NumSamples.addr = alloca i32, align 4
  %InverseTransform.addr = alloca i32, align 4
  %RealIn.addr = alloca float*, align 8
  %ImagIn.addr = alloca float*, align 8
  %RealOut.addr = alloca float*, align 8
  %ImagOut.addr = alloca float*, align 8
  %NumBits = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %n = alloca i32, align 4
  %BlockSize = alloca i32, align 4
  %BlockEnd = alloca i32, align 4
  %angle_numerator = alloca double, align 8
  %tr = alloca double, align 8
  %ti = alloca double, align 8
  %delta_angle = alloca double, align 8
  %sm2 = alloca double, align 8
  %sm1 = alloca double, align 8
  %cm2 = alloca double, align 8
  %cm1 = alloca double, align 8
  %w = alloca double, align 8
  %ar = alloca [3 x double], align 16
  %ai = alloca [3 x double], align 16
  %temp = alloca double, align 8
  %denom = alloca double, align 8
  store i32 %NumSamples, i32* %NumSamples.addr, align 4
  store i32 %InverseTransform, i32* %InverseTransform.addr, align 4
  store float* %RealIn, float** %RealIn.addr, align 8
  store float* %ImagIn, float** %ImagIn.addr, align 8
  store float* %RealOut, float** %RealOut.addr, align 8
  store float* %ImagOut, float** %ImagOut.addr, align 8
  store double 0x401921FB54442D18, double* %angle_numerator, align 8
  %call = call i32 @IsPowerOfTwo(i32 %NumSamples)
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %call1 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %0, i8* getelementptr inbounds ([52 x i8], [52 x i8]* @.str, i32 0, i32 0), i32 %NumSamples)
  call void @exit(i32 1) #4
  unreachable

if.end:                                           ; preds = %entry
  %tobool2 = icmp ne i32 %InverseTransform, 0
  br i1 %tobool2, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  store double 0xC01921FB54442D18, double* %angle_numerator, align 8
  br label %if.end4

if.end4:                                          ; preds = %if.then3, %if.end
  %1 = bitcast float* %RealIn to i8*
  call void @CheckPointer(i8* %1, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.1, i32 0, i32 0))
  %2 = bitcast float* %RealOut to i8*
  call void @CheckPointer(i8* %2, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i32 0, i32 0))
  %3 = bitcast float* %ImagOut to i8*
  call void @CheckPointer(i8* %3, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.3, i32 0, i32 0))
  %call5 = call i32 @NumberOfBitsNeeded(i32 %NumSamples)
  store i32 %call5, i32* %NumBits, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %cond.end, %if.end4
  %4 = phi float* [ %8, %cond.end ], [ %ImagIn, %if.end4 ]
  %5 = phi i32 [ %inc, %cond.end ], [ 0, %if.end4 ]
  %cmp = icmp ult i32 %5, %NumSamples
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call6 = call i32 @ReverseBits(i32 %5, i32 %call5)
  store i32 %call6, i32* %j, align 4
  %idxprom = zext i32 %5 to i64
  %arrayidx = getelementptr inbounds float, float* %RealIn, i64 %idxprom
  %6 = load float, float* %arrayidx, align 4
  %idxprom7 = zext i32 %call6 to i64
  %arrayidx8 = getelementptr inbounds float, float* %RealOut, i64 %idxprom7
  store float %6, float* %arrayidx8, align 4
  %cmp9 = icmp eq float* %4, null
  br i1 %cmp9, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body
  br label %cond.end

cond.false:                                       ; preds = %for.body
  %arrayidx11 = getelementptr inbounds float, float* %ImagIn, i64 %idxprom
  %7 = load float, float* %arrayidx11, align 4
  %conv = fpext float %7 to double
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %8 = phi float* [ null, %cond.true ], [ %ImagIn, %cond.false ]
  %cond = phi double [ 0.000000e+00, %cond.true ], [ %conv, %cond.false ]
  %conv12 = fptrunc double %cond to float
  %arrayidx14 = getelementptr inbounds float, float* %ImagOut, i64 %idxprom7
  store float %conv12, float* %arrayidx14, align 4
  %inc = add i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 1, i32* %BlockEnd, align 4
  store i32 2, i32* %BlockSize, align 4
  br label %for.cond15

for.cond15:                                       ; preds = %for.end110, %for.end
  %9 = phi float* [ %15, %for.end110 ], [ %ImagOut, %for.end ]
  %10 = phi i32 [ %16, %for.end110 ], [ 1, %for.end ]
  %11 = phi float* [ %19, %for.end110 ], [ %RealOut, %for.end ]
  %12 = phi i32 [ %17, %for.end110 ], [ %NumSamples, %for.end ]
  %13 = phi i32 [ %shl, %for.end110 ], [ 2, %for.end ]
  %cmp16 = icmp ule i32 %13, %12
  br i1 %cmp16, label %for.body18, label %for.end112

for.body18:                                       ; preds = %for.cond15
  %14 = load double, double* %angle_numerator, align 8
  %conv19 = uitofp i32 %13 to double
  %div = fdiv double %14, %conv19
  store double %div, double* %delta_angle, align 8
  %mul = fmul double -2.000000e+00, %div
  %call20 = call double @sin(double %mul) #5
  store double %call20, double* %sm2, align 8
  %sub21 = fsub double -0.000000e+00, %div
  %call22 = call double @sin(double %sub21) #5
  store double %call22, double* %sm1, align 8
  %call24 = call double @cos(double %mul) #5
  store double %call24, double* %cm2, align 8
  %call26 = call double @cos(double %sub21) #5
  store double %call26, double* %cm1, align 8
  %mul27 = fmul double 2.000000e+00, %call26
  store double %mul27, double* %w, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond28

for.cond28:                                       ; preds = %for.end107, %for.body18
  %15 = phi float* [ %24, %for.end107 ], [ %9, %for.body18 ]
  %16 = phi i32 [ %48, %for.end107 ], [ %13, %for.body18 ]
  %17 = phi i32 [ %.pre, %for.end107 ], [ %12, %for.body18 ]
  %18 = phi i32 [ %add109, %for.end107 ], [ 0, %for.body18 ]
  %19 = phi float* [ %27, %for.end107 ], [ %11, %for.body18 ]
  %cmp29 = icmp ult i32 %18, %17
  br i1 %cmp29, label %for.body31, label %for.end110

for.body31:                                       ; preds = %for.cond28
  %20 = load double, double* %cm2, align 8
  %arrayidx32 = getelementptr inbounds [3 x double], [3 x double]* %ar, i64 0, i64 2
  store double %20, double* %arrayidx32, align 16
  %21 = load double, double* %cm1, align 8
  %arrayidx33 = getelementptr inbounds [3 x double], [3 x double]* %ar, i64 0, i64 1
  store double %21, double* %arrayidx33, align 8
  %22 = load double, double* %sm2, align 8
  %arrayidx34 = getelementptr inbounds [3 x double], [3 x double]* %ai, i64 0, i64 2
  store double %22, double* %arrayidx34, align 16
  %23 = load double, double* %sm1, align 8
  %arrayidx35 = getelementptr inbounds [3 x double], [3 x double]* %ai, i64 0, i64 1
  store double %23, double* %arrayidx35, align 8
  store i32 %18, i32* %j, align 4
  store i32 0, i32* %n, align 4
  br label %for.cond36

for.cond36:                                       ; preds = %for.body39, %for.body31
  %24 = phi float* [ %39, %for.body39 ], [ %15, %for.body31 ]
  %25 = phi i32 [ %inc105, %for.body39 ], [ %18, %for.body31 ]
  %26 = phi i32 [ %inc106, %for.body39 ], [ 0, %for.body31 ]
  %27 = phi float* [ %44, %for.body39 ], [ %19, %for.body31 ]
  %cmp37 = icmp ult i32 %26, %10
  br i1 %cmp37, label %for.body39, label %for.end107

for.body39:                                       ; preds = %for.cond36
  %28 = load double, double* %w, align 8
  %29 = load double, double* %arrayidx33, align 8
  %mul41 = fmul double %28, %29
  %30 = load double, double* %arrayidx32, align 16
  %sub43 = fsub double %mul41, %30
  %arrayidx44 = getelementptr inbounds [3 x double], [3 x double]* %ar, i64 0, i64 0
  store double %sub43, double* %arrayidx44, align 16
  store double %29, double* %arrayidx32, align 16
  store double %sub43, double* %arrayidx33, align 8
  %31 = load double, double* %arrayidx35, align 8
  %mul50 = fmul double %28, %31
  %32 = load double, double* %arrayidx34, align 16
  %sub52 = fsub double %mul50, %32
  %arrayidx53 = getelementptr inbounds [3 x double], [3 x double]* %ai, i64 0, i64 0
  store double %sub52, double* %arrayidx53, align 16
  store double %31, double* %arrayidx34, align 16
  store double %sub52, double* %arrayidx35, align 8
  %add = add i32 %25, %10
  store i32 %add, i32* %k, align 4
  %idxprom59 = zext i32 %add to i64
  %arrayidx60 = getelementptr inbounds float, float* %27, i64 %idxprom59
  %33 = load float, float* %arrayidx60, align 4
  %conv61 = fpext float %33 to double
  %mul62 = fmul double %sub43, %conv61
  %arrayidx65 = getelementptr inbounds float, float* %24, i64 %idxprom59
  %34 = load float, float* %arrayidx65, align 4
  %conv66 = fpext float %34 to double
  %mul67 = fmul double %sub52, %conv66
  %sub68 = fsub double %mul62, %mul67
  store double %sub68, double* %tr, align 8
  %35 = load float, float* %arrayidx65, align 4
  %conv72 = fpext float %35 to double
  %mul73 = fmul double %sub43, %conv72
  %36 = load float, float* %arrayidx60, align 4
  %conv77 = fpext float %36 to double
  %mul78 = fmul double %sub52, %conv77
  %add79 = fadd double %mul73, %mul78
  store double %add79, double* %ti, align 8
  %idxprom80 = zext i32 %25 to i64
  %arrayidx81 = getelementptr inbounds float, float* %27, i64 %idxprom80
  %37 = load float, float* %arrayidx81, align 4
  %conv82 = fpext float %37 to double
  %sub83 = fsub double %conv82, %sub68
  %conv84 = fptrunc double %sub83 to float
  store float %conv84, float* %arrayidx60, align 4
  %38 = load i32, i32* %j, align 4
  %idxprom87 = zext i32 %38 to i64
  %39 = load float*, float** %ImagOut.addr, align 8
  %arrayidx88 = getelementptr inbounds float, float* %39, i64 %idxprom87
  %40 = load float, float* %arrayidx88, align 4
  %conv89 = fpext float %40 to double
  %41 = load double, double* %ti, align 8
  %sub90 = fsub double %conv89, %41
  %conv91 = fptrunc double %sub90 to float
  %42 = load i32, i32* %k, align 4
  %idxprom92 = zext i32 %42 to i64
  %arrayidx93 = getelementptr inbounds float, float* %39, i64 %idxprom92
  store float %conv91, float* %arrayidx93, align 4
  %43 = load double, double* %tr, align 8
  %44 = load float*, float** %RealOut.addr, align 8
  %arrayidx95 = getelementptr inbounds float, float* %44, i64 %idxprom87
  %45 = load float, float* %arrayidx95, align 4
  %conv96 = fpext float %45 to double
  %add97 = fadd double %conv96, %43
  %conv98 = fptrunc double %add97 to float
  store float %conv98, float* %arrayidx95, align 4
  %46 = load float, float* %arrayidx88, align 4
  %conv101 = fpext float %46 to double
  %add102 = fadd double %conv101, %41
  %conv103 = fptrunc double %add102 to float
  store float %conv103, float* %arrayidx88, align 4
  %inc105 = add i32 %38, 1
  store i32 %inc105, i32* %j, align 4
  %47 = load i32, i32* %n, align 4
  %inc106 = add i32 %47, 1
  store i32 %inc106, i32* %n, align 4
  br label %for.cond36

for.end107:                                       ; preds = %for.cond36
  %48 = load i32, i32* %BlockSize, align 4
  %49 = load i32, i32* %i, align 4
  %add109 = add i32 %49, %48
  store i32 %add109, i32* %i, align 4
  %.pre = load i32, i32* %NumSamples.addr, align 4
  br label %for.cond28

for.end110:                                       ; preds = %for.cond28
  store i32 %16, i32* %BlockEnd, align 4
  %shl = shl i32 %16, 1
  store i32 %shl, i32* %BlockSize, align 4
  br label %for.cond15

for.end112:                                       ; preds = %for.cond15
  %50 = load i32, i32* %InverseTransform.addr, align 4
  %tobool113 = icmp ne i32 %50, 0
  br i1 %tobool113, label %if.then114, label %if.end133

if.then114:                                       ; preds = %for.end112
  %conv115 = uitofp i32 %12 to double
  store double %conv115, double* %denom, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond116

for.cond116:                                      ; preds = %for.body119, %if.then114
  %51 = phi double [ %54, %for.body119 ], [ %conv115, %if.then114 ]
  %52 = phi i32 [ %inc131, %for.body119 ], [ 0, %if.then114 ]
  %cmp117 = icmp ult i32 %52, %12
  br i1 %cmp117, label %for.body119, label %for.end132

for.body119:                                      ; preds = %for.cond116
  %idxprom120 = zext i32 %52 to i64
  %arrayidx121 = getelementptr inbounds float, float* %11, i64 %idxprom120
  %53 = load float, float* %arrayidx121, align 4
  %conv122 = fpext float %53 to double
  %div123 = fdiv double %conv122, %51
  %conv124 = fptrunc double %div123 to float
  store float %conv124, float* %arrayidx121, align 4
  %54 = load double, double* %denom, align 8
  %55 = load i32, i32* %i, align 4
  %idxprom125 = zext i32 %55 to i64
  %56 = load float*, float** %ImagOut.addr, align 8
  %arrayidx126 = getelementptr inbounds float, float* %56, i64 %idxprom125
  %57 = load float, float* %arrayidx126, align 4
  %conv127 = fpext float %57 to double
  %div128 = fdiv double %conv127, %54
  %conv129 = fptrunc double %div128 to float
  store float %conv129, float* %arrayidx126, align 4
  %inc131 = add i32 %55, 1
  store i32 %inc131, i32* %i, align 4
  br label %for.cond116

for.end132:                                       ; preds = %for.cond116
  br label %if.end133

if.end133:                                        ; preds = %for.end132, %for.end112
  ret void
}

declare i32 @IsPowerOfTwo(i32) #1

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) #1

; Function Attrs: noreturn nounwind
declare void @exit(i32) #2

; Function Attrs: nounwind uwtable
define internal void @CheckPointer(i8* %p, i8* %name) #0 {
entry:
  %p.addr = alloca i8*, align 8
  %name.addr = alloca i8*, align 8
  store i8* %p, i8** %p.addr, align 8
  store i8* %name, i8** %name.addr, align 8
  %cmp = icmp eq i8* %p, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %call = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %0, i8* getelementptr inbounds ([35 x i8], [35 x i8]* @.str.4, i32 0, i32 0), i8* %name)
  call void @exit(i32 1) #4
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i32 @NumberOfBitsNeeded(i32) #1

declare i32 @ReverseBits(i32, i32) #1

; Function Attrs: nounwind
declare double @sin(double) #3

; Function Attrs: nounwind
declare double @cos(double) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn nounwind }
attributes #5 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 261206)"}
