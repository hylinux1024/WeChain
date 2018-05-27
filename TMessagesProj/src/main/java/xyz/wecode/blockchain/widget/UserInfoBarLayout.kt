package xyz.wecode.blockchain.widget

import android.annotation.TargetApi
import android.content.Context
import android.util.AttributeSet
import android.widget.FrameLayout
import kotlinx.android.synthetic.main.user_info_bar_layout.view.*
import org.telegram.messenger.ContactsController
import org.telegram.messenger.R
import org.telegram.tgnet.TLRPC
import org.telegram.ui.Components.AvatarDrawable

class UserInfoBarLayout : FrameLayout {

    constructor(context: Context?) : super(context) {
        init(null, 0, 0)
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        init(attrs, 0, 0)
    }

    constructor(context: Context?, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        init(attrs, defStyleAttr, 0)

    }

    @TargetApi(21)
    constructor(context: Context?, attrs: AttributeSet?, defStyleAttr: Int, defStyleRes: Int) : super(context, attrs, defStyleAttr, defStyleRes) {
        init(attrs, defStyleAttr, defStyleRes)
    }

    private fun init(attrs: AttributeSet?, defStyleAttr: Int, defStyleRes: Int) {
        inflate(context, R.layout.user_info_bar_layout, this)
        val a = context.obtainStyledAttributes(attrs, R.styleable.UserInfoBarLayout, defStyleAttr, defStyleRes)
        val userName = a.getString(R.styleable.UserInfoBarLayout_userName)
        tvUserName.text = userName
        val userId = a.getString(R.styleable.UserInfoBarLayout_userId)
        tvUserId.text = userId
        val avatar = a.getResourceId(R.styleable.UserInfoBarLayout_userIcon, R.mipmap.ic_default_avatar)
        ivUserAvatar.setImageResource(avatar)

        val qrcode = a.getResourceId(R.styleable.UserInfoBarLayout_qrCodeIcon, R.mipmap.ic_qr_code)
        ivQRCode.setImageResource(qrcode)
        a.recycle()
        isClickable = true
    }

    fun setUserInfo(user: TLRPC.User) {
        val name = ContactsController.formatName(user.first_name, user.last_name)
        tvUserName.text = name
        tvUserId.text = context?.getString(R.string.telegram_id, user.id)
        val drawable = AvatarDrawable(user, true)
        drawable.setDrawCircle(false)
        ivUserAvatar.setImage(user.photo.photo_small, "60_60", drawable)
//        ivUserAvatar.setImageDrawable(drawable)
    }

//    fun getUserAvatar(): ImageView {
//        return ivUserAvatar
//    }
//
//    fun getQRCodeIcon(): ImageView {
//        return ivQRCode
//    }

}