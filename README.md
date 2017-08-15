# ProtocolLogHelper
帮助打印RPC框架中接口的调用日志,使用变参模板实现任意类型任意个数的请求,回包,以及单个任意类型的返回值

>   auto logPrinter = ProtocolLogHelper::Reqs(req1, req2, req3).Rsps(rsp1, rsp2, rsp3).Ret(retCode, \__FUNCTION__);

   其中 req1-3表示你的request结构体实例,可以是任意类型任意个数, rsp1-3表示你的返回结果结构体实例,可以是任意类型任意个数,retCode是单个任意类型的接口返回值.\__FUNCTION\__是当前函数名,如果接口名称和函数名称不一样,这里可以自行传入正确的接口名称字符串.
   
   logPrinter会持有rsp1-3实例以及retCode实例的引用,在logPrinter的生命周期内,需要保证传入的rsp和retcode实例有效,在定义logPrinter之后,可以随意修改rsp1-3和retCode,因为logPrinter持有的是引用,所以后面的修改都将影响最终logPrinter析构时的最终返回结果的打印.
   
   如果你的接口中使用异步方式调用了其他接口,并且在异步接口返回之前无法返回当前接口,那么则需要使用下面的代码初始化日志打印器:

>   auto logPrinter = ProtocolLogHelper::Reqs(req1, req2, req3).Rsps(rsp1, rsp2, rsp3).RetPtr(retCode, \__FUNCTION__);

   和上面的例子唯一的不同是, 最后的Ret函数变成了RetPtr函数,返回值是一个智能指针,那么在整个异步调用过程中,都需要持有logPrinter,保证其不被析构,通常做法是把logPrinter作为callback类中的一个成员变量,通过decttype可以提取logPrinter的类型, 如果callback是一个lambda表达式,那么只需要把logPrinter填入caputer list中即可, 例如: [logPrinter](...){...}
