function doGet(e) {
  var uid = e.parameter.uid;
  Logger.log("Received UID: " + uid);  // Ghi log để kiểm tra
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("NguoiDung");
  var data = sheet.getDataRange().getValues();

  for (var i = 1; i < data.length; i++) {
    if (data[i][0] == uid) {
      var name = data[i][1];
      var balance = parseFloat(data[i][2]);
      var topup = parseFloat(data[i][3]);

      var newBalance = balance + topup;
      sheet.getRange(i + 1, 3).setValue(newBalance);
      sheet.getRange(i + 1, 4).setValue(0); // Reset topup column after adding

      return ContentService.createTextOutput(name + "|" + newBalance);
    }
  }
  
  return ContentService.createTextOutput("UNKNOWN");
}

function doPost(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("NguoiDung");
  const action = e.parameter.action;
  const uid = e.parameter.uid;
  const sodu = e.parameter.sodu;

  if (action == "update") {
    const data = sheet.getDataRange().getValues();
    for (let i = 1; i < data.length; i++) {
      if (data[i][0] == uid) {
        sheet.getRange(i + 1, 3).setValue(sodu);  // Cập nhật số dư vào cột 3
        return ContentService.createTextOutput("OK");
      }
    }
    return ContentService.createTextOutput("UNKNOWN");
  }

  return ContentService.createTextOutput("Invalid action");
}
