define dso_local i32 @example(i32 %0) {
  %2 = icmp sgt i32 %0, 0, !ID !1
  br i1 %2, label %3, label %5, !ID !2

3:                                       ; preds = %1
  %4 = add nsw i32 %0, 1, !ID !3
  br label %7, !ID !4

5:                                       ; preds = %1
  %6 = sub nsw i32 %0, 1, !ID !5
  br label %7, !ID !6

7:                                       ; preds = %5, %3
  %.0 = phi i32 [ %4, %3 ], [ %6, %5 ], !ID !7
  ret i32 %.0, !ID !8
}


