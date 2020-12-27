#pragma once
#include <void/assert.h>
#include "fsm_types.h"
namespace cph {
	namespace hsm {

		namespace details {
			enum EventResultDetails { EventHandled = 0, EventIgnored, EventTransition, EventReturnParent }; //return by handler
		}
		using EventResult = details::EventResultDetails;

		class Hsm;
		typedef  EventResult(Hsm::* StateHandler) /* name of the pointer-to-function type */
		(const fsm::Event* e);  /* argument list */



		template<typename T>
		static constexpr cph::hsm::StateHandler as_state(cph::hsm::EventResult(T::* f)(
		            cph::fsm::Event const* e)) {
			return static_cast<cph::hsm::StateHandler>(f);
		}


		class Hsm: cph::fsm::FsmBase {
			private:
				constexpr static uint8_t MAX_NEST_DEPTH = 7;

				template <class T>
				using StateHandlerDerived = cph::hsm::EventResult(T::*)(cph::fsm::Event const* e);
			protected:
				StateHandler m_state; // current active state (state-variable)
			public:
				//return event
				constexpr inline EventResult eventHandled() {
					return details::EventHandled;
				}
				constexpr inline EventResult eventIgnored() {
					return  details::EventIgnored;
				}

				template <typename T>
				constexpr inline EventResult eventSuper(StateHandlerDerived<T> super) {
					this->m_state = as_state<T>(super);
					return  details::EventReturnParent;
				}

				template <typename T>
				constexpr inline EventResult eventTransition(StateHandlerDerived<T> target) {
					this->m_state = as_state<T>(target);
					return  details::EventTransition;
				}

				//invoke state function
				template < fsm::Signal signal>
				constexpr inline EventResult invoke(StateHandler& state) { //invoke strategy
					//return (this->*state)({ signal });/* invoke state handler s */
					return (this->*state)(&FsmBase::_ReservedEvents[signal]);/* invoke state handler s */
				}

				constexpr inline EventResult invoke(StateHandler& state, cph::fsm::Event const& e) {
					return (this->*state)(&e);/* invoke state handler s */
				}

				constexpr inline void invoke_enter(StateHandler& state) {
					if (invoke<cph::fsm::SIG_ENTRY>(state) == details::EventHandled) {
						/* software tracing instrumentation for state enter */\
					}
				}

				constexpr inline bool invoke_exit(StateHandler& state) {
					if (invoke<cph::fsm::SIG_EXIT>(state) == details::EventHandled) {
						/* software tracing instrumentation for state enter */
						return true;
					}

					return false;
				}

				void init(cph::fsm::Event const* e = (cph::fsm::Event const*)0) {
					StateHandler t = &Hsm::top; /* HSM starts in the top state */
					/* the top-most initial transition must be taken */
					assert(invoke(m_state, *e) == details::EventTransition);

					do { /* drill into the target. . . */
						StateHandler path[MAX_NEST_DEPTH];
						int8_t ip = (int8_t)0; /* transition entry path index */
						path[0] = this->m_state; /* save the target of the initial transition */
						invoke<cph::fsm::SIG_EMPTY>(m_state);

						while (this->m_state != t) {
							path[++ip] = this->m_state;
							invoke<cph::fsm::SIG_EMPTY>(this->m_state);
						}

						this->m_state = path[0]; /* restore the target of the initial tran. */
						/* entry path must not overflow */
						assert(ip < (int8_t)MAX_NEST_DEPTH);

						do { /* retrace the entry path in reverse (desired) order. . . */
							invoke_enter(path[ip]); /* enter path [ip] */
						} while ((--ip) >= (int8_t)0);

						t = path[0]; /* current state becothiss the new source */
					} while (invoke<cph::fsm::SIG_INIT>(t) == details::EventTransition);

					this->m_state = t;
				}


				void dispatch(cph::fsm::Event const& e) {
					StateHandler path[MAX_NEST_DEPTH];
					StateHandler t;
					StateHandler s;
					EventResult r;
					t = this->m_state; /* save the current state */

					do { /* process the Event hierarchically. . . */
						s = this->m_state;
						r = invoke(s, e);/* invoke state handler s */
					} while (r == details::EventReturnParent);

					if (r == details::EventTransition) { /* transition taken? */
						int8_t ip = (int8_t)(-1); /* transition entry path index */
						int8_t iq; /* helper transition entry path index */
						path[0] = this->m_state; /* save the target of the transition */
						path[1] = t;

						while (t != s) { /* exit current state to transition source s. . . */
							if (invoke_exit(t)) { /*exit handled? */
								invoke<cph::fsm::SIG_EMPTY>(t); /* find superstate of t */
							}

							t = this->m_state; /* this->m_state holds the superstate */
						}

						//. . .
						t = path[0]; /* target of the transition */

						if (s == t) { /* (a) check source==target (transition to self) */
							invoke_exit(s); /* exit the source */
							ip = (int8_t)0; /* enter the target */
						} else { /*1======================================================================*/
							(void)invoke<cph::fsm::SIG_EMPTY>(t); /* superstate of target */
							t = this->m_state;

							if (s == t) { /* (b) check source==target->super */
								ip = (int8_t)0; /* enter the target */
							} else { /*2======================================================================*/
								(void)invoke<cph::fsm::SIG_EMPTY>(s); /* superstate of src */
								/* (c) check source->super==target->super */

								if (this->m_state == t) {
									invoke_exit(s); /* exit the source */
									ip = (int8_t)0; /* enter the target */
								} else { /*3======================================================================*/
									/* (d) check source->super==target */
									if (this->m_state == path[0]) {
										invoke_exit(s); /* exit the source */
									} else { /*4======================================================================*/
										/* (e) check rest of source==target->super->super..
										* and store the entry path along the way
										*/
										iq = (int8_t)0; /* indicate that LCA not found */
										ip = (int8_t)1; /* enter target and its superstate */
										path[1] = t; /* save the superstate of target */
										t = this->m_state; /* save source->super */
										/* find target->super->super */
										r = invoke<cph::fsm::SIG_EMPTY>(path[1]);

										while (r == details::EventReturnParent) { /*5==============================*/
											path[++ip] = this->m_state; /* store the entry path */

											if (this->m_state == s) { /* is it the source? */
												iq = (int8_t)1; /* indicate that LCA found */
												/* entry path must not overfl*/
												assert(ip < (int8_t)MAX_NEST_DEPTH);
												--ip; /* do not enter the source */
												r = details::EventHandled; /* terminate the loop */
											} else { /* it is not the source, keep going up */
												r = invoke<cph::fsm::SIG_EMPTY>(this->m_state);
											}
										} /*-5==============================*/

										if (iq == (int8_t)0) { /* the LCA not found yet? */
											/*5==============================*/
											/* entry path must not overflow */
											assert(ip < (int8_t)MAX_NEST_DEPTH);
											invoke_exit(s); /* exit the source */
											/* (f) check the rest of source->super
											* == target->super->super...
											*/
											iq = ip;
											r = details::EventIgnored; /* indicate LCA NOT found */

											do {
												if (t == path[iq]) { /* is this the LCA? */
													r = details::EventHandled; /* indicate LCA found */
													ip = (int8_t)(iq - 1); /*do not enter LCA*/
													iq = (int8_t)(-1); /* terminate the loop */
												} else {
													--iq; /* try lower superstate of target */
												}
											} while (iq >= (int8_t)0);

											if (r != details::EventHandled) { /* LCA not found yet? */
												/*6==============================*/
												/* (g) check each source->super->...
												* for each target->super...
												*/
												r = details::EventIgnored; /* keep looping */

												do { /*7==============================*/
													/* exit t unhandled? */
													if (invoke<cph::fsm::SIG_EXIT>(t) == details::EventHandled) {
														invoke<cph::fsm::SIG_EMPTY>(t);
													}

													t = this->m_state; /* set to super of t */
													iq = ip;

													do { /*8==============================*/
														if (t == path[iq]) { /* is this LCA? */
															/* do not enter LCA */
															ip = (int8_t)(iq - 1);
															iq = (int8_t)(-1); /*break inner */
															r = details::EventHandled; /*break outer */
														} else {
															--iq;
														}
													} while (iq >= (int8_t)0); //-8
												} while (r != details::EventHandled);/*-7==============================*/
											}/*-6==============================*/
										}/*-5==============================*/
									}/*-4==============================*/
								}/*-3==============================*/
							}/*-2==============================*/
						}/*-1==============================*/

						/* retrace the entry path in reverse (desired) order... */
						for (; ip >= (int8_t)0; --ip) {
							invoke_enter(path[ip]); /* enter path [ip] */
						}

						t = path[0]; /* stick the target into register */
						this->m_state = t; /* update the current state */

						/* drill into the target hierarchy... */
						while (invoke<cph::fsm::SIG_INIT>(t) == details::EventTransition) {
							ip = (int8_t)0;
							path[0] = this->m_state;
							invoke<cph::fsm::SIG_EMPTY>(this->m_state); /* find superstate */

							while (this->m_state != t) {
								path[++ip] = this->m_state;
								invoke<cph::fsm::SIG_EMPTY>(this->m_state); /* find superstate */
							}

							this->m_state = path[0];
							/* entry path must not overflow */
							assert(ip < (int8_t)MAX_NEST_DEPTH);

							do { /* retrace the entry path in reverse (correct) order... */
								invoke_enter(path[ip]); /* enter path [ip] */
							} while ((--ip) >= (int8_t)0);

							t = path[0];
						}//while
					}

					//...
					this->m_state = t; //set new state or restore
				}

				//uint8_t isIn(QHsmState state) {}
				constexpr EventResult top(fsm::Event const*) {
					return eventIgnored();
				}
			protected:
				Hsm(StateHandler initial) : m_state(initial) {} // protected ctor
		};

	}
}
