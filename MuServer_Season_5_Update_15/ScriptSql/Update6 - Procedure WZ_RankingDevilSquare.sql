CREATE Procedure [dbo].[WZ_RankingDevilSquare] 
@Account varchar(10),
@Name varchar(10),
@Score int
AS
BEGIN

SET NOCOUNT ON
SET XACT_ABORT ON

IF NOT EXISTS (SELECT Name FROM RankingDevilSquare WHERE Name=@Name)
BEGIN

	 INSERT INTO RankingDevilSquare (Name,Score) VALUES (@Name,@Score)

END
ELSE
BEGIN

	UPDATE RankingDevilSquare SET Score=Score+@Score WHERE Name=@Name

END

SET NOCOUNT OFF
SET XACT_ABORT OFF

END
