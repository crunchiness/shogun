#!/usr/bin/env python
from tools.load import LoadMatrix

lm=LoadMatrix()
data = lm.load_numbers('../data/fm_train_real.dat')

parameter_list = [[data,20],[data,30]]

def converter_laplacianeigenmaps_modular (data,k):
	try:
		from shogun.Features import RealFeatures
		from shogun.Converter import LaplacianEigenmaps
		
		features = RealFeatures(data)
			
		converter = LaplacianEigenmaps()
		converter.set_target_dim(1)
		converter.set_k(k)
		converter.set_tau(20.0)
		converter.apply(features)
		
		return features
	except ImportError:
		print('No Eigen3 available')

if __name__=='__main__':
	print('LaplacianEigenmaps')
	converter_laplacianeigenmaps_modular(*parameter_list[0])

