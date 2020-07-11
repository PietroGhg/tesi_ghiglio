; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: norecurse nounwind readnone sspstrong uwtable
define dso_local i32 @f(i32 %0, i32 %1) local_unnamed_addr #0 !dbg !9 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !14, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 %1, metadata !15, metadata !DIExpression()), !dbg !16
  %3 = icmp sgt i32 %0, %1, !dbg !17
  br i1 %3, label %8, label %4, !dbg !19

4:                                                ; preds = %2, %4
  %5 = phi i32 [ %7, %4 ], [ %0, %2 ]
  call void @llvm.dbg.value(metadata i32 %5, metadata !14, metadata !DIExpression()), !dbg !16
  %6 = icmp sgt i32 %5, %1, !dbg !20
  %7 = add nsw i32 %5, 1, !dbg !22
  call void @llvm.dbg.value(metadata i32 %7, metadata !14, metadata !DIExpression()), !dbg !16
  br i1 %6, label %8, label %4, !dbg !24, !llvm.loop !25

8:                                                ; preds = %4, %2
  %9 = phi i32 [ %0, %2 ], [ %5, %4 ]
  %10 = add nsw i32 %9, %1, !dbg !27
  call void @llvm.dbg.value(metadata i32 %10, metadata !14, metadata !DIExpression()), !dbg !16
  ret i32 %10, !dbg !28
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { norecurse nounwind readnone sspstrong uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 10.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/home/pietro/Documents/Polimi/Quinto anno/tesi/code/test")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{i32 7, !"PIE Level", i32 2}
!8 = !{!"clang version 10.0.0 "}
!9 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !12, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15}
!14 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !1, line: 1, type: !12)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !9, file: !1, line: 1, type: !12)
!16 = !DILocation(line: 0, scope: !9)
!17 = !DILocation(line: 2, column: 10, scope: !18)
!18 = distinct !DILexicalBlock(scope: !9, file: !1, line: 2, column: 8)
!19 = !DILocation(line: 2, column: 8, scope: !9)
!20 = !DILocation(line: 6, column: 17, scope: !21)
!21 = distinct !DILexicalBlock(scope: !18, file: !1, line: 5, column: 9)
!22 = !DILocation(line: 7, column: 14, scope: !23)
!23 = distinct !DILexicalBlock(scope: !21, file: !1, line: 6, column: 22)
!24 = !DILocation(line: 6, column: 9, scope: !21)
!25 = distinct !{!25, !24, !26}
!26 = !DILocation(line: 8, column: 9, scope: !21)
!27 = !DILocation(line: 0, scope: !18)
!28 = !DILocation(line: 11, column: 5, scope: !9)
