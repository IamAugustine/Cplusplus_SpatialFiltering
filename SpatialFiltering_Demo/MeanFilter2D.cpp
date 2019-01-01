#include "pch.h"
#include "MeanFilter2D.h"
#include <functional>
#include <numeric>
MeanFilter2D::MeanFilter2D()
{
}

MeanFilter2D::MeanFilter2D(byte size)
{
	Kernel = new MeanKernel(size);
}

MeanFilter2D::MeanFilter2D(byte sizeX, byte sizeY)
{
	Kernel = new MeanKernel(sizeY, sizeX);
}


MeanFilter2D::~MeanFilter2D()
{
	
}

void MeanFilter2D::Apply(ushort * imageIn, int height, int width, ushort * imageOut)
{
	ImageHeight = height;
	ImageWidth = width;
	deque<int> dataSegment;
	size_t threadCount = GetCPUCoreNumber();
	blockHeight = ImageHeight / threadCount + Kernel->BoundaryV * 2;
	ushort** fltdSegmentts = new ushort*[threadCount];
	ushort** blockSegments = SegmentImage(imageIn, threadCount);
	for (size_t i = 0; i < threadCount; i++)
	{
		fltdSegmentts[i] = new ushort[blockHeight*ImageWidth];
		thread t(&MeanFilter2D::FilterBlock, this, blockSegments[i], fltdSegmentts[i]);//"this" is the key for C2839
		if (t.joinable()) t.join();
	}
	delete blockSegments;

	ReconstructImage(fltdSegmentts, imageOut, threadCount, blockHeight);

	delete fltdSegmentts;

}

void MeanFilter2D::FilterBlock(ushort * imageIn, ushort * imageOut)
{
	std::deque<float> kernelDeque;
	//std::deque<ushort> tempDeque;
	for (size_t rowIndex = Kernel->BoundaryV; rowIndex < blockHeight - Kernel->BoundaryV; rowIndex++)
	{
		kernelDeque = InitializeDeque(imageIn, rowIndex);
		//tempDeque = kernelDeque;
		int newClmIndex = 0;
		float mean = 0;
		//std::nth_element(tempDeque.begin(), tempDeque.begin() + tempDeque.size() / 2, tempDeque.end());
		imageOut[rowIndex * ImageWidth + newClmIndex] = (ushort) (std::accumulate(kernelDeque.begin(), kernelDeque.end(), 0) / kernelDeque.size());
		newClmIndex++;
		while (newClmIndex <= ImageWidth)
		{
			KernelMoveRight(imageIn, rowIndex, newClmIndex + Kernel->BoundaryH, kernelDeque);
			imageOut[rowIndex * ImageWidth + newClmIndex] = (ushort)(std::accumulate(kernelDeque.begin(), kernelDeque.end(), 0) / kernelDeque.size());
			newClmIndex++;
		}
	}
}

void MeanFilter2D::KernelMoveRight(ushort * imgIn, int rowIndex, int clmIndexToAdd, deque<float>& tmp)
{
	tmp.pop_front();
	int newClmIndex = clmIndexToAdd >= ImageWidth ? 2 * ImageWidth - clmIndexToAdd : clmIndexToAdd;//Mirror boundary
	float mean = 0;
	for (int i = -1 * Kernel->BoundaryV; i <= Kernel->BoundaryV; i++)
	{
		int kernelRowIndex = (rowIndex + i)*ImageWidth + newClmIndex;
		mean += (float)(imgIn[kernelRowIndex])/ Kernel->BoundaryV;
		tmp.push_back(mean);
	}
}

deque<float> MeanFilter2D::InitializeDeque(ushort * imgIn, const int y)
{
	deque<float> originKernel;
	for (int kx = -1 * Kernel->BoundaryH; kx <= Kernel->BoundaryH; kx++)
	{
		float mean = 0;
		for (int ky = -1 * Kernel->BoundaryV; ky <= 1 * Kernel->BoundaryV; ky++)
		{
			int pixel00Index = (ky + Kernel->BoundaryV + y)*ImageWidth + abs(kx);
			mean += (float)(imgIn[pixel00Index])/ Kernel->BoundaryV;
		}
		originKernel.push_back(mean);
	}
	return originKernel;
}
