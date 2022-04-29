# Regression Testing Data 

## Reconstruct

Regression test data for reconstruct are stored in an Azure Blob container at https://corelibrariescidata.blob.core.windows.net/regression-test-data

The regression test suite contains 100 iset files:

* 40 images from  `N:/BIOVOL2/Occasional_data/Precision` randomly selected from the subfolders `02-05-2019` and `26-03-2019`. These have been recompressed with the JPEG-LS codec

* 41 images with Thermal Calibration V2 provided by Anna. `6982cf5c-7aa5-4ebb-8c8f-804ae4c18106` fails measurement due
  to an insufficient tumour size

* 19 images for evaluating stereo reconstruction improvements provided by Ernesto (One synthetic tumour image),
    recompressed with JPEG-LS
  
### Syncing the data locally

1. Download [azcopy](https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10)
2. Get a SAS token from the Azure portal, or use another means of [authentication](https://docs.microsoft.com/en-us/azure/storage/common/storage-use-azcopy-v10#choose-how-youll-provide-authorization-credentials)
3. run
   ```
   mkdir regression-test-data
   azcopy sync "https://corelibrariescidata.blob.core.windows.net/regression-test-data?<SAS token>" regression-test-data 
   ```
  
## Stereo Calibration

Datasets for testing stereo calibration are located at 
https://corelibrariescidata.blob.core.windows.net/calibration-test-data/stereo

