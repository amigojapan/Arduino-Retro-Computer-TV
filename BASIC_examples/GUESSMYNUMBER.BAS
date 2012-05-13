1 REM Guess my number
2 PRINT "1 to 100..."
3 X=1
4 Y=100
5 N=RND
6 PRINT "Your guess? "
7 INPUT G
8 IF G=N THEN 11
9 IF G<N THEN 13
10 IF G>N THEN 15
11 PRINT "Got it!"
12 END
13 PRINT "Too low"
14 GOTO 6
15 PRINT "Too high"
16 GOTO 6
