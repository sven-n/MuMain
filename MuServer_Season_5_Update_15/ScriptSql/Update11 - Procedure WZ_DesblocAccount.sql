CREATE Procedure [dbo].[WZ_DesblocAccount] 
@Account varchar(10)
AS
BEGIN

SET NOCOUNT ON
SET XACT_ABORT ON

UPDATE MEMB_INFO SET bloc_code = 0 where memb___id = @Account and bloc_code = 1 and bloc_expire < GETDATE()

UPDATE Character SET CtlCode = 0 where AccountID = @Account and CtlCode = 1 and bloc_expire < GETDATE()

SET NOCOUNT OFF
SET XACT_ABORT OFF

END



