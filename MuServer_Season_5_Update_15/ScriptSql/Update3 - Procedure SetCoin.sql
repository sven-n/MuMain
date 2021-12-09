Create Procedure [dbo].[WZ_SetCoin] 
@Account varchar(10),
@Name varchar(10),
@Value1 int,
@Value2 int,
@Value3 int
AS
BEGIN

SET NOCOUNT ON
SET XACT_ABORT ON

-- Tipo 1: Update Cash | Wcoin
	IF @Value1 > 0 
	BEGIN
		--UPDATE CashShopData SET WCoinC = WCoinC + @Value1 WHERE AccountID = @Account
		--Para Season 4
		UPDATE MEMB_INFO SET Cash = Cash + @Value1 WHERE memb___id = @Account 
	END
	
-- Tipo 2: Update Gold | WcoinP
	IF @Value2 > 0  
	BEGIN
		--UPDATE CashShopData SET WCoinP = WCoinP + @Value2 WHERE AccountID = @Account
		--Para Season 4
		UPDATE MEMB_INFO SET Gold = Gold + @Value2 WHERE memb___id = @Account 
	END

-- Tipo 3: Update PcPoints | GoblinPoints
	IF @Value3 > 0 
	BEGIN
		--UPDATE CashShopData SET GoblinPoint = GoblinPoint + @Value3 WHERE AccountID = @Account
		--Para Season 4
		UPDATE PcPointdata SET PcPoint = PcPoint + @Value3 WHERE AccountID = @Account 
	END
	

SET NOCOUNT OFF
SET XACT_ABORT OFF

END


GO
