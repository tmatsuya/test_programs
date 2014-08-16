#set style histogram clustered
set style fill solid border lc rgb "black"
set yrange [0:15000000]
#set xrange [0:9]
#set lmargin 8
#set rmargin 2
#set tmargin 1.5
set bmargin 6.0

#set xtics offset first 0.1,0
set xlabel "RSS"
set ylabel "Receive (PPS)"
#plot "bypass.dat" using 2:xtic(1) with histogram notitle
set boxwidth 0.8
plot "bypass.dat" using ($0*4+0):2 with boxes title "ByPass", "" using ($0*4+1):3:xtic(1) with boxes title "Normal"
replot

# 出力フォーマットとオプションの指定
set terminal postscript eps color enhanced 25
# 出力ファイル名の指定
set output "bypass.eps"
# グラフ再描画
set size 1.5, 1.5
replot

# フォーマットと出力のリセット
set output
