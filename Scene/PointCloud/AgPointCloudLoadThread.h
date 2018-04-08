#pragma once

#include <memory>
#include <vector>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wundef"
#endif
#include <tbb/tbb_thread.h>
#ifdef __clang__
#undef true
#undef false
#undef bool
#pragma clang diagnostic pop
#endif

#include <utility/RCMutex.h>
#include <utility/RCIThreadWorker.h>

namespace ambergris {

	struct ScanContainerID;

	class WorkerThread
	{
	public:

		WorkerThread();

		virtual                             ~WorkerThread();

		virtual bool                        Start();

		virtual void                        Stop();

		virtual bool                        IsRunning() const;

		virtual void                        Join();

	protected:

		static void                         Run(WorkerThread * worker);


	private:

		virtual void                        doTask() = 0;

		// Private data members.
		bool                                mEndThread;
		bool                                mRunning;
		tbb::tbb_thread*                    mpThread;
	};


	class AgPointCloudRequests : public WorkerThread
	{
	public:
		enum class Status
		{
			Inactive,       //Request set hasn't been provided since last clear
			Active,         //Request set has been given
			Complete        //All requests in set are loaded
		};

		AgPointCloudRequests();
		~AgPointCloudRequests();

		Status                          GetStatus() const;

		void                            Clear();
		virtual bool                    Start();
		void                            SetWorkingSet(const std::vector<ScanContainerID>& newRequest);
		void                            SetSortNodes(bool val);

	private:

		virtual void                    doTask();

		std::vector<ScanContainerID>    mRequestSet;

		bool                            mInterruptLoading;
		volatile Status                 mStatus;

		std::unique_ptr<RealityComputing::Utility::Threading::RCMutex> mRequestSetMutex;
		bool                            mSortNodes;
	};

}