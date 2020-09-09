#ifndef BOSS_MSG_TASKCOMPLETION_HPP
#define BOSS_MSG_TASKCOMPLETION_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::TaskCompletion
 *
 * @brief signals completion of some task.
 *
 * @desc This is generally used in testing.
 */
struct TaskCompletion {
	/* Some description of the task that was completed.  */
	std::string comment;
	/* Usually the address of the module that completed a task.  */
	void *identifier;
	/* True if the task failed.  */
	bool failed;
};

}}

#endif /* !defined(BOSS_MSG_TASKCOMPLETION_HPP) */
