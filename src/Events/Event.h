#ifndef EVENT_H
#define EVENT_H
#include "Base/KeyCodes.h"
#include "Base/MouseCodes.h"

namespace viz {
	enum eventType {
		noEvent = 0,
		windowClose, windowResize,
		appUpdate, appRender,
		keyPress, keyRelease,
		mouseButtonPress, mouseButtonRelease, mouseMove, mouseScroll,
		particleHit
	};

	enum eventCategory {
		noEventCat = 0,
		eventCatWindow,
		eventCatApp,
		eventCatKey,
		eventCatMouse,
		eventCatParticle
	};

	struct EventData {
		EventData() = default;
		std::pair<float, float> floatPairedData;
		std::pair<unsigned int, unsigned int> uintPairedData;
		std::pair<std::string, unsigned int> stringUIntPairedData;
		viz::key::keyCodes keyButton;
		viz::mouse::mouseCodes mouseButton;
	};

	class event{
	public:
		bool handled = false;

		virtual eventType getEventType() const = 0;
		virtual eventCategory getEventCat() const = 0;
		virtual EventData* getData()const  = 0;

		virtual std::string toString() const = 0;

		virtual ~event() = default;
		event() = default;
	};

	class eventSender {
		template<typename T>
		using fn = std::function<bool(T&)>;

	public:
		eventSender(event& e) : m_Event(e) {}

		template<typename T>
		void Send(fn<T> fun) {
			if (m_Event.getEventType() == T::GetStaticType()) {
				m_Event.handled = fun(*(T*)&m_Event);
			}
		}

		std::string toString() const { return "eventSender"; }
	private:
		event& m_Event;
	};
}

#endif