package xyz.wecode.blockchain.ui

import android.content.Context
import android.view.View
import org.telegram.messenger.R
import org.telegram.ui.ActionBar.BaseFragment

class DiscoverFragment : BaseFragment() {
    override fun createView(context: Context?): View {
        fragmentView = View.inflate(context, R.layout.discover_fragment, null)

        return fragmentView
    }

    override fun canShowBottomTabLayout(): Boolean = true
}