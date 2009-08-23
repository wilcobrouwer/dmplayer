#include "thread_group2.h"
#include <boost/foreach.hpp>

using namespace util;
using namespace boost;
using namespace std;

thread_group2::thread_group2() {
// 	done = false;
// 	thread_manager_thread = shared_ptr<thread>(new thread(bind(&thread_group2::thread_manager, this)));
}

thread_group2::~thread_group2() {
	join_all();
}

void thread_group2::join_all() {
	mutex::scoped_lock clock(thread_creation_mutex); // disallow creation of new threads
	vector<shared_ptr<thread> > my_threads;
	{
	mutex::scoped_lock lock(threads_mutex);
	my_threads = threads;
	}
	BOOST_FOREACH(shared_ptr<thread> pt, my_threads) {
		pt->join();
	}
}

void thread_group2::thread_runner(const function0<void>& threadfunc) {
	threadfunc();
	{
	mutex::scoped_lock lock(threads_mutex);
	for(std::vector<shared_ptr<thread> >::iterator i = threads.begin(); i != threads.end() ; ++i) {
		if((*i)->get_id() == this_thread::get_id()) {
			threads.erase(i);
			break;
		}
	}
	} //FIXME: Thread is not *really* exited yet, don't know how bad this is...
}

thread* thread_group2::create_thread(const function0<void>& threadfunc) {
	mutex::scoped_lock lock(threads_mutex);
	mutex::scoped_lock clock(thread_creation_mutex);
	shared_ptr<thread> t = shared_ptr<thread>(new thread(bind(&thread_group2::thread_runner, this, threadfunc)));
	threads.push_back(t);
	return t.get();
}