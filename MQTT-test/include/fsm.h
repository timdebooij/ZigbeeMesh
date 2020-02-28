
#include <array>
#include <functional>

namespace smartlight
{
	template <typename State, typename Symbol, size_t NUM_STATE, size_t NUM_SYMBOLS>
	class FSM {
	public:
		struct Transition {
			std::function<void()> handler;
			State target;
		};

		size_t time;
		Symbol nextSymbol;
		bool checkTimer = false;

		template <typename Func>
		void addTransition(Symbol symbol, State from, State target, Func handler)
		{
			Transition transition { handler, target };
			this->stt_[from][symbol] = transition;
		}

		void raise(Symbol input)
		{
			Transition& transition = this->stt_[this->currentState_][input];

			transition.handler();
			this->currentState_ = transition.target;
		}

		void initTimer(int time, Symbol next){
			this->time = millis() + time;
			this->nextSymbol = next;
			this->checkTimer = true;
		}

	private:
		std::array<std::array<Transition, NUM_SYMBOLS>, NUM_STATE> stt_;
		State currentState_;

	};
}
