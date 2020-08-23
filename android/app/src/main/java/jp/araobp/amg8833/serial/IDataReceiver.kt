package jp.araobp.amg8833.serial

interface IDataReceiver {

    fun onAmg8833Data(data: Amg8833Data)

}