
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H
#include "Event.h"
#include "Base/MouseCodes.h"

namespace viz {
	class mouseEvent : public event{
	public:
		virtual ~mouseEvent() = default;

		eventCategory getEventCat() const override {
			return eventCategory::eventCatMouse;
		}

		static eventType GetStaticType() { return viz::eventType::noEvent;  }
	};

	class mouseEventPressed : public mouseEvent {
	public:
		viz::mouse::mouseCodes getMousePressed() const { return mousePressed->mouseButton; }

		mouseEventPressed(const mouse::mouseCodes& mouse) { mousePressed = new EventData; mousePressed->mouseButton = mouse; }
		mouseEventPressed(const mouse::mouseCodes&& mouse) { mousePressed = new EventData; mousePressed->mouseButton = mouse; }

		static eventType GetStaticType() { return eventType::mouseButtonPress; }
		eventType getEventType() const override { return eventType::mouseButtonPress; }
		EventData* getData() const override { return mousePressed;  }

		std::string toString() const {
			std::stringstream ss;
			ss << "mouseEventPressed: " << getMousePressed();
			return ss.str();
		}

		~mouseEventPressed() { delete mousePressed; }
	private:
		EventData* mousePressed;
	};

	class mouseEventReleased : public mouseEvent {
	public:
		viz::mouse::mouseCodes getMouseReleased() const { return mouseReleased->mouseButton; }

		static eventType GetStaticType() { return eventType::mouseButtonRelease; }
		eventType getEventType() const override { return eventType::mouseButtonRelease; }

		mouseEventReleased(const mouse::mouseCodes& mouse) { mouseReleased = new EventData; mouseReleased->mouseButton = mouse; }
		mouseEventReleased(const mouse::mouseCodes&& mouse) { mouseReleased = new EventData; mouseReleased->mouseButton = mouse; }
		EventData* getData() const override { return mouseReleased; }

		std::string toString() const {
			std::stringstream ss;
			ss << "mouseEventReleased: " << getMouseReleased();
			return ss.str();
		}

		~mouseEventReleased() { delete mouseReleased; }
	private:
		EventData* mouseReleased;
	};


	class mouseEventMoved : public mouseEvent {
	public:
		mouseEventMoved(const float&& x, const float&& y) {
			mousePos = new EventData;
			mousePos->floatPairedData = { x, y };
		}

		mouseEventMoved(const float& x, const float& y){
			mousePos = new EventData;
			mousePos->floatPairedData = { x, y };
		}

		static eventType GetStaticType() { return eventType::mouseMove; }
		eventType getEventType() const override { return eventType::mouseMove; }
		EventData* getData() const override { return mousePos; }
		

		std::string toString() const {
			std::stringstream ss;
			ss << "mouseEventMoved: (" << mousePos->floatPairedData.first << ", " << mousePos->floatPairedData.second << ")\n";
			return ss.str();
		}

		std::pair<float, float> getMousePos() {
			return mousePos->floatPairedData;
		}

		~mouseEventMoved() { delete mousePos;  }
	private:
		EventData* mousePos;
	};

	class mouseEventScrolled : public mouseEvent {
	public:
		mouseEventScrolled(const float&& xOffset, const float&& yOffset) {
			scrollOffset = new EventData;
			scrollOffset->floatPairedData = { xOffset, yOffset };
		}

		mouseEventScrolled(const float& xOffset, const float& yOffset) {
			scrollOffset = new EventData;
			scrollOffset->floatPairedData = { xOffset, yOffset };
		}

		static eventType GetStaticType() { return eventType::mouseScroll; }
		eventType getEventType() const override { return eventType::mouseScroll; }
		EventData* getData() const override { return scrollOffset; }


		std::string toString() const {
			std::stringstream ss;
			ss << "mouseEventScrolled: (" << scrollOffset->floatPairedData.first << ", " << scrollOffset->floatPairedData.second << ")\n";
			return ss.str();
		}

		std::pair<float, float> getMouseScrollOffset() {
			return scrollOffset->floatPairedData;
		}

		~mouseEventScrolled() { delete scrollOffset;  }
	private:
		EventData* scrollOffset;
	};
}

#endif