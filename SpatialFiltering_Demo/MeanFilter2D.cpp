#include "pch.h"
#include "MeanFilter2D.h"
#include <functional>
#include <numeric>
//MeanFilter2D::MeanFilter2D()
//{
//}

//MeanFilter2D::MeanFilter2D(byte size)
//{
//	Kernel = new MeanKernel(size);
//}

MeanFilter2D::MeanFilter2D(byte sizeX, byte sizeY)
{
	Kernel = new MeanKernel(sizeY, sizeX);
}


MeanFilter2D::~MeanFilter2D()
{
	
}

//void MeanFilter2D::Apply(  ushort * imageIn, int height, int width, ushort * imageOut)
//{
//	ImageHeight = height;
//	ImageWidth = width;
//	deque<int> dataSegment;
//	size_t threadCount = GetCPUCoreNumber();
//	blockHeight = ImageHeight / threadCount + Kernel->RadiusV * 2;
//	ushort** fltdSegmentts = new ushort*[threadCount];
//	ushort** blockSegments = SegmentImage(imageIn, threadCount);
//	for (size_t i = 0; i < threadCount; i++)
//	{
//		fltdSegmentts[i] = new ushort[blockHeight*ImageWidth];
//		thread t(&MeanFilter2D::FilterBlock, this, blockSegments[i], fltdSegmentts[i]);//"this" is the key for C2839
//		if (t.joinable()) t.join();
//	}
//	delete blockSegments;
//
//	Re ructImage(fltdSegmentts, imageOut, threadCount, blockHeight);
//
//	delete fltdSegmentts;
//
//}

void MeanFilter2D::FilterBlock(  ushort * imageIn, ushort * imageOut)
{
	std::deque<float> kernelDeque;
	//std::deque<ushort> tempDeque;
	for (size_t rowIndex = bndBufferHeight; rowIndex < blockHeight - bndBufferHeight; rowIndex++)
	{
		kernelDeque = InitializeDeque2(imageIn, rowIndex);
		//tempDeque = kernelDeque;
		int newClmIndex = 0;
		float mean = 0;
		//std::nth_element(tempDeque.begin(), tempDeque.begin() + tempDeque.size() / 2, tempDeque.end());
		float v = (std::accumulate(kernelDeque.begin(), kernelDeque.end(), 0) / kernelDeque.size());
		imageOut[rowIndex * ImageWidth + newClmIndex] = (ushort)(v+0.5);
		float meanToRemove = kernelDeque[0];
		newClmIndex++;
		while (newClmIndex < ImageWidth)
		{
			KernelMoveRight(imageIn, rowIndex, newClmIndex + Kernel->RadiusH, kernelDeque);
			v += (float)(kernelDeque.back() - meanToRemove) / Kernel->HorizontalSize;
			imageOut[rowIndex * ImageWidth + newClmIndex] = (ushort)(v+0.5);
			meanToRemove = kernelDeque[0];
			newClmIndex++;
		}
	}
}

void MeanFilter2D::KernelMoveRight(  ushort * imgIn, int rowIndex, int clmIndexToAdd, deque<float>& tmp)
{
	tmp.pop_front();
	int newClmIndex = clmIndexToAdd >= ImageWidth ? 2 * ImageWidth - clmIndexToAdd-1 : clmIndexToAdd;//Mirror boundary
	float mean = 0;
	for (int i = -1 * Kernel->RadiusV; i <= Kernel->RadiusV; i++)
	{
		int kernelRowIndex = (rowIndex + i)*ImageWidth + newClmIndex;
		mean += (float)(imgIn[kernelRowIndex])/ Kernel->VerticalSize;
		
	}
	tmp.push_back(mean);
}

deque<float> MeanFilter2D::InitializeDeque2(  ushort * imgIn,   int y)
{
	deque<float> originKernel;
	for (int kx = -1 * Kernel->RadiusH; kx <= Kernel->RadiusH; kx++)
	{
		float mean = 0;
		for (int ky = -1 * Kernel->RadiusV; ky <= 1 * Kernel->RadiusV; ky++)
		{
			int pixel00Index = (ky + y)*ImageWidth + abs(kx);
			mean += (float)(imgIn[pixel00Index])/ Kernel->VerticalSize;
		}
		originKernel.push_back(mean);
	}
	return originKernel;
}

void MeanFilter2D::ProcessingBlocks(ushort ** blocksIn, byte blockHeight, byte threadCount, ushort ** blocksOut)
{
	for (size_t i = 0; i < threadCount; i++)
	{
		blocksOut[i] = new ushort[blockHeight*ImageWidth];
		thread t(fun,this, blocksIn[i], blocksOut[i]);//"this" is the key for C2893
		if (t.joinable()) t.join();
	}
}
