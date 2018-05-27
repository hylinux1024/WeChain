package xyz.wecode.blockchain.ui

import android.content.Context
import android.os.Bundle
import android.view.View
import org.telegram.messenger.ContactsController
import org.telegram.messenger.LocaleController
import org.telegram.messenger.R
import org.telegram.messenger.UserConfig
import org.telegram.messenger.query.StickersQuery
import org.telegram.ui.ActionBar.BaseFragment
import org.telegram.ui.ChatActivity
import org.telegram.ui.Components.AvatarDrawable
import org.telegram.ui.SettingsActivity
import org.telegram.ui.StickersActivity
import xyz.wecode.blockchain.widget.LinearItemLayout
import xyz.wecode.blockchain.widget.UserInfoBarLayout

class MeFragment : BaseFragment() {

    private val userInfoBarLayout by lazy { find<UserInfoBarLayout>(R.id.userInfoBar) }
    private val itemFavorites by lazy { find<LinearItemLayout>(R.id.itemFavorites) }
    private val settingsView by lazy { find<LinearItemLayout>(R.id.itemSettings) }
    private val stickerView by lazy { find<LinearItemLayout>(R.id.itemSticker) }

    override fun canShowBottomTabLayout(): Boolean = true

    override fun createView(context: Context?): View {
        fragmentView = View.inflate(context, R.layout.me_fragment, null)
        actionBar.setTitle(LocaleController.getString("AppName", R.string.AppName))
        userInfoBarLayout.setUserInfo(UserConfig.getCurrentUser())
        setListener()
        return fragmentView
    }

    private fun setListener() {
        settingsView.setOnClickListener {
            presentFragment(SettingsActivity())
        }
        stickerView.setOnClickListener {
            presentFragment(StickersActivity(StickersQuery.TYPE_IMAGE))
        }
        itemFavorites.setOnClickListener {
            //收藏到本地的消息
            val args = Bundle()
            args.putInt("user_id", UserConfig.getClientUserId())
            presentFragment(ChatActivity(args))
        }
    }

}