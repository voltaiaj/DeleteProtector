cd C:\Drivers\DeleteProtector
sc create DeleteProtector type= filesys start= demand binPath= "C:\Drivers\DeleteProtector\DeleteProtector.sys"
sc start DeleteProtector