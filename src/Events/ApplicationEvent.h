#ifndef APPEVENT_H
#define APPEVENT_H
#include "Event.h"

namespace viz {
	class windowEventClose : public event{
		public:

		static eventType GetStaticType() { return eventType::windowClose; }
		eventType getEventType() const override { return eventType::windowClose; }
		eventCategory getEventCat() const override { return eventCategory::eventCatApp; }
		EventData* getData() const override { return NULL; }

		std::string toString() const override {
			return "windowEventClose";
		}
	};

	class windowEventResize : public event{
	public:
		windowEventResize(const unsigned int&& width, const unsigned int&& height) {
			windowSize = new EventData;
			windowSize->uintPairedData = { width, height };
		}

		windowEventResize(const unsigned int& width, const unsigned int& height) {
			windowSize = new EventData;
			windowSize->uintPairedData = { width, height };
		}

		static eventType GetStaticType() { return eventType::windowResize; }
		eventType getEventType() const override { return eventType::windowResize;}
		eventCategory getEventCat() const override { return eventCategory::eventCatApp; }
		EventData* getData() const override { return windowSize; }

		std::string toString() const override{
			std::stringstream ss;
			ss << "windowEventResize: (" << windowSize->uintPairedData.first << ", " << windowSize->uintPairedData.second << ")\n";
			return ss.str();
		}

		~windowEventResize() override {delete windowSize;}
	private:
		EventData* windowSize;
	};

	class appRender : public event {
	public:

		static eventType GetStaticType() { return eventType::appRender; }
		eventType getEventType() const override { return eventType::appRender; }
		eventCategory getEventCat() const override { return eventCategory::eventCatApp; }
		EventData* getData() const override { return NULL; }

		std::string toString() const override {
			return "applicationRender";
		}
	};

	class appUpdate : public event {
	public:

		static eventType GetStaticType() { return eventType::appUpdate; }
		eventType getEventType() const override { return eventType::appUpdate; }
		eventCategory getEventCat() const override { return eventCategory::eventCatApp; }


		std::string toString() const override {
			return "applicationUpdate";
		}
	};
}

#endif