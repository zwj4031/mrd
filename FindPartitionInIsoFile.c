#include "MyRamDisk.h"

///�ҳ�������Ϣ�����ص���4��������
EFI_STATUS
	FindPartitionInIsoFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINTN					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINTN					*BootStartAddr,
		OUT		UINT64					*BootSize
		)
		{
			CDROM_VOLUME_DESCRIPTOR     *VolDescriptor=NULL;
			ELTORITO_CATALOG            *TempCatalog=NULL;
			UINTN						DescriptorSize=RAM_DISK_BLOCK_SIZE;	
			UINTN						DbrImageSize=2;
			UINT16						DbrImageSizeBuffer;
			UINTN	i;
			VolDescriptor = AllocatePool (DescriptorSize);
			if (VolDescriptor == NULL) {
				return EFI_NOT_FOUND;
				}
			//�жϾ�
			FileHandleSetPosition(FileDiskFileHandle,CD_BOOT_SECTOR*RAM_DISK_BLOCK_SIZE); 	
			FileHandleRead(FileDiskFileHandle,&DescriptorSize,VolDescriptor);
			if(	VolDescriptor->Unknown.Type!=CDVOL_TYPE_STANDARD||
				CompareMem (VolDescriptor->BootRecordVolume.SystemId, CDVOL_ELTORITO_ID, sizeof (CDVOL_ELTORITO_ID) - 1) != 0){
				FreePool(VolDescriptor);
				return EFI_NOT_FOUND;
				}
			//�ж�����Ŀ¼	
			TempCatalog = (ELTORITO_CATALOG*)VolDescriptor;
			FileHandleSetPosition(FileDiskFileHandle,*((UINT32*)VolDescriptor->BootRecordVolume.EltCatalog)*RAM_DISK_BLOCK_SIZE); 	
			FileHandleRead(FileDiskFileHandle,&DescriptorSize,TempCatalog);	
			if( TempCatalog[0].Catalog.Indicator!=ELTORITO_ID_CATALOG){
				FreePool(VolDescriptor);
				return EFI_NOT_FOUND;
				}
			for(i=0;i<64;i++){
				if( TempCatalog[i].Catalog.Indicator==ELTORITO_ID_CATALOG&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE&&
					TempCatalog[i+1].Boot.LoadSegment==0x7c0){
					*NoBootStartAddr	=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*NoBootSize	 		=TempCatalog[i+1].Boot.SectorCount*FLOPPY_DISK_BLOCK_SIZE;
					
					}
				
				
				if( TempCatalog[i].Section.Indicator==ELTORITO_ID_SECTION_HEADER_FINAL&&
					TempCatalog[i].Section.PlatformId==IS_EFI_SYSTEM_PARTITION&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE ){
						
					*BootStartAddr	=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*BootSize		=TempCatalog[i+1].Boot.SectorCount*FLOPPY_DISK_BLOCK_SIZE;
					//��Щ���̵�ӳ���С�趨����ȷ��̫С�Ͷ�ȡ����ӳ���dbr
					FileHandleSetPosition(FileDiskFileHandle,*BootStartAddr+0x13); 	
					FileHandleRead(FileDiskFileHandle,&DbrImageSize,&DbrImageSizeBuffer);
					
					DbrImageSize=DbrImageSizeBuffer*FLOPPY_DISK_BLOCK_SIZE;
					*BootSize=*BootSize>DbrImageSize?*BootSize:DbrImageSize;
						
					//��Ȼ̫С����Ϊ�̶�ֵ	
					if(*BootSize<BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE){
						*BootSize=BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE;
						}
					FreePool(VolDescriptor);	
					return EFI_SUCCESS;
					}
				
				}				

	
		FreePool(VolDescriptor);		
		return 	EFI_NOT_FOUND;		
		}