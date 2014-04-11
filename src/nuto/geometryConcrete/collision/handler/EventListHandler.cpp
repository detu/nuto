/*
 * EventListHandler.cpp
 *
 *  Created on: 3 Mar 2014
 *      Author: ttitsche
 */

#include "nuto/geometryConcrete/collision/handler/EventListHandler.h"
#include "nuto/geometryConcrete/collision/collidables/CollidableBase.h"
#include "nuto/geometryConcrete/collision/handler/SubBoxHandler.h"
#include "nuto/geometryConcrete/WallTime.h"

NuTo::EventListHandler::EventListHandler()
		:
				mTimeUpdate(0.),
				mTimeErase(0.),
				mTimeAdd(0.),
				mTimeRebuild(0.),
				mTimeBarrier(0.),
				mNSphereCollisions(0),
				mNWallCollisions(0),
				mNWallTransfers(0)
{
	mEvents.clear();
}


NuTo::EventListHandler::~EventListHandler()
{
	mEvents.clear();
}

void NuTo::EventListHandler::PrintEvents()
{
	std::cout << std::endl << "======== GLOBAL EVENT LIST ========" << std::endl;
	for (auto event : mEvents)
		std::cout << event << std::endl;

	std::cout << "===================================" << std::endl << std::endl;
}

void NuTo::EventListHandler::Clear()
{
	mEvents.clear();
}

void NuTo::EventListHandler::AddEvent(const double rTime,
		CollidableBase& rCollidable1, CollidableBase& rCollidable2, int rType)
{
	if (rTime < 0 || rTime > mTimeBarrier)
		return;

	auto insert = mEvents.insert(new Event(rTime, &rCollidable2, &rCollidable1, rType));
	(*insert.first).AddLocalEvent();

//	Event* tmp = new Event(rTime, &rCollidable2, &rCollidable1, rType);
//	tmp->AddLocalEvent();
//	mEvents.push_back(tmp);
}

void NuTo::EventListHandler::DeleteOldEvents(Event::LocalEvents& rOldEvents)
{
	for (unsigned int iOldEvent = 0; iOldEvent < rOldEvents.size(); ++iOldEvent) {
		mEvents.erase(*rOldEvents[iOldEvent]);
	}
}

const double NuTo::EventListHandler::GetNextEventTime()
{
	if (mEvents.begin() == mEvents.end())
		return Event::EVENTNULL;

	return (*mEvents.begin()).GetTime();
}

void NuTo::EventListHandler::PrintStatistics(double rTimeTotal)
{
	std::ostringstream statistics;
	double timeSum = mTimeUpdate + mTimeErase + mTimeAdd + mTimeRebuild;
	statistics.setf(std::ios_base::fixed, std::ios_base::floatfield);
	statistics.precision(4);
	statistics << "Time Update: " << mTimeUpdate << " | " << mTimeUpdate / rTimeTotal * 100 << "%" << std::endl;
	statistics << "Time Erase:  " << mTimeErase << " | " << mTimeErase / rTimeTotal * 100 << "%" << std::endl;
	statistics << "Time Add:    " << mTimeAdd << " | " << mTimeAdd / rTimeTotal * 100 << "%" << std::endl;
	statistics << "Time Rebuild:" << mTimeRebuild << " | " << mTimeRebuild / rTimeTotal * 100 << "%" << std::endl;
	statistics << "Time Sum:    " << timeSum << " | " << timeSum / rTimeTotal * 100 << "%" << std::endl;
	statistics << "Time Total:  " << std::setw(6) << rTimeTotal << std::endl;

	long nEvents = mNSphereCollisions + mNWallCollisions + mNWallTransfers;

	statistics << "Total Events: " << nEvents << std::endl;
	statistics << 100. * mNSphereCollisions / nEvents << "% SphereCollisions" << std::endl;
	statistics << 100. * mNWallCollisions / nEvents << "% WallCollisions" << std::endl;
	statistics << 100. * mNWallTransfers / nEvents << "% WallTransfers" << std::endl;

	std::cout << statistics.str();
}


const int NuTo::EventListHandler::GetEventListSize()
{
	return mEvents.size();
}

void NuTo::EventListHandler::PerformNextEvent()
{

	const Event* nextEvent = new Event(*(mEvents.begin()));

	switch (nextEvent->GetType())
	{
	case Event::EventType::SphereCollision:
		mNSphereCollisions++;
		break;
	case Event::EventType::WallCollision:
		mNWallCollisions++;
		break;
	case Event::EventType::WallTransfer:
		mNWallTransfers++;
		break;
	default:
		break;
	}

	double timeTmp;

	timeTmp = WallTime::Get();
	nextEvent->PerformCollision();
	mTimeUpdate += WallTime::Get() - timeTmp;

	timeTmp = WallTime::Get();
	nextEvent->EraseOldEvents(*this);
	mTimeErase += WallTime::Get() - timeTmp;

	timeTmp = WallTime::Get();
	nextEvent->AddNewEvents(*this);
	mTimeAdd += WallTime::Get() - timeTmp;

	delete nextEvent;

}

double NuTo::EventListHandler::SetTimeBarrier(double rTimeBarrier, SubBoxHandler& rSubBoxes)
{
	mTimeBarrier = rTimeBarrier;

	double sTime = WallTime::Get();

	mEvents.clear();

	// rebuild the event list
	auto& boxes = rSubBoxes.GetSubBoxes();
	for (unsigned int iBox = 0; iBox < boxes.size(); ++iBox)
	{
		auto& collidables = boxes[iBox]->GetCollidables();
		for (unsigned int iCollidable = 0; iCollidable < collidables.size(); ++iCollidable)
		{
			boxes[iBox]->CreateEvents(*this, *collidables[iCollidable]);
		}
	}


	double timeRebuild = WallTime::Get() - sTime;
	mTimeRebuild += timeRebuild;


	std::stringstream ostream;
	ostream.setf(std::ios_base::scientific);


	ostream << "New time barrier: " << std::setprecision(8) << rTimeBarrier;
	ostream << " created  "  << mEvents.size() << " events in " << timeRebuild << " s." << std::endl;

	std::cout << ostream.str();

	return timeRebuild;
}
