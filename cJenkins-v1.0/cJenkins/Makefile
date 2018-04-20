#Makefile

#定义变量
cc=g++ -g
srcPath=./src
incPath=./inc
objPath=./obj
cpps=$(shell ls $(srcPath) | grep ".cpp")
obj=$(cpps:%.cpp=$(objPath)/%.o)
binPath=./bin
libs=-lmyodbc5 -lpthread

#关键字
vpath %.cpp $(srcPath)

#make all
all : $(binPath)/cJenkins
	@echo "make completed..."

#部署
$(binPath)/cJenkins : $(objPath)/cJenkins
	ln -f $(objPath)/cJenkins $(binPath)/cJenkins
	@echo "可执行程序部署完成..."

#连接
$(objPath)/cJenkins : $(obj)
	@echo "程序编译完成..."
	$(cc) $(libs) $^ -o $@
	@echo "程序连接完成..."

#编译
$(objPath)/%.o : %.cpp $(incPath)/*.h
	$(cc) -c $< -o $@ -I$(incPath)

#清理
clean :
	rm -f $(objPath)/* $(binPath)/cJenkins
	@echo "编译连接部署中间文件清理完成..."

##