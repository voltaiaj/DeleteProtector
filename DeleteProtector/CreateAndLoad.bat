cd C:\Drivers\DeleteProtector

sc delete DeleteProtector
sc create DeleteProtector type= filesys start= demand binPath= "C:\Drivers\DeleteProtector\DeleteProtector.sys"
sc start DeleteProtector