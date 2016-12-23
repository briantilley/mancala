all:
	javac source/com/briantilley/*.java -d out

stage:
	git add -A
	git status

do:
	@java -cp out com.briantilley.TestDriver

clean:
	rm -rf out/*.class
