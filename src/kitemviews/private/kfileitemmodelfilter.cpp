/*
 * SPDX-FileCopyrightText: 2011 Janardhan Reddy <annapareddyjanardhanreddy@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kfileitemmodelfilter.h"

#include <QRegularExpression>

#include <algorithm>

#include <KFileItem>

KFileItemModelFilter::KFileItemModelFilter()
    : m_useRegExp(false)
    , m_regExp(nullptr)
    , m_lowerCasePattern()
    , m_pattern()
    , m_hiddenFilesShown(true)
    , m_hiddenWhitelistEnabled(false)
    , m_hiddenWhitelist()
    , m_hiddenWhitelistRegExps()
{
}

KFileItemModelFilter::~KFileItemModelFilter()
{
    delete m_regExp;
    m_regExp = nullptr;
}

void KFileItemModelFilter::setPattern(const QString &filter)
{
    m_pattern = filter;
    m_lowerCasePattern = filter.toLower();

    if (filter.contains(QLatin1Char('*')) || filter.contains(QLatin1Char('?')) || filter.contains(QLatin1Char('['))) {
        if (!m_regExp) {
            m_regExp = new QRegularExpression();
            m_regExp->setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        m_regExp->setPattern(QRegularExpression::wildcardToRegularExpression(filter));
        m_useRegExp = m_regExp->isValid();
    } else {
        m_useRegExp = false;
    }
}

QString KFileItemModelFilter::pattern() const
{
    return m_pattern;
}

void KFileItemModelFilter::setMimeTypes(const QStringList &types)
{
    m_mimeTypes = types;
}

QStringList KFileItemModelFilter::mimeTypes() const
{
    return m_mimeTypes;
}

void KFileItemModelFilter::setExcludeMimeTypes(const QStringList &types)
{
    m_excludeMimeTypes = types;
}

QStringList KFileItemModelFilter::excludeMimeTypes() const
{
    return m_excludeMimeTypes;
}

void KFileItemModelFilter::setHiddenFilesShown(bool shown)
{
    m_hiddenFilesShown = shown;
}

bool KFileItemModelFilter::hiddenFilesShown() const
{
    return m_hiddenFilesShown;
}

void KFileItemModelFilter::setHiddenFilesWhitelistEnabled(bool enabled)
{
    m_hiddenWhitelistEnabled = enabled;
}

bool KFileItemModelFilter::hiddenFilesWhitelistEnabled() const
{
    return m_hiddenWhitelistEnabled;
}

void KFileItemModelFilter::setHiddenFilesWhitelist(const QStringList &patterns)
{
    m_hiddenWhitelist = patterns;
    updateHiddenWhitelistRegExps();
}

QStringList KFileItemModelFilter::hiddenFilesWhitelist() const
{
    return m_hiddenWhitelist;
}

void KFileItemModelFilter::updateHiddenWhitelistRegExps()
{
    m_hiddenWhitelistRegExps.clear();
    for (const QString &pattern : m_hiddenWhitelist) {
        const QString trimmed = pattern.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        // Check if pattern contains wildcards
        if (trimmed.contains(QLatin1Char('*')) || trimmed.contains(QLatin1Char('?')) || trimmed.contains(QLatin1Char('['))) {
            QRegularExpression regExp(QRegularExpression::wildcardToRegularExpression(trimmed));
            regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            if (regExp.isValid()) {
                m_hiddenWhitelistRegExps.append(regExp);
            }
        } else {
            // Exact match - create a simple regex that matches the exact string
            QRegularExpression regExp(QStringLiteral("^%1$").arg(QRegularExpression::escape(trimmed)));
            regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            m_hiddenWhitelistRegExps.append(regExp);
        }
    }
}

bool KFileItemModelFilter::matchesHiddenWhitelist(const KFileItem &item) const
{
    const QString name = item.text();
    for (const QRegularExpression &regExp : m_hiddenWhitelistRegExps) {
        if (regExp.match(name).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool KFileItemModelFilter::hasSetFilters() const
{
    return (!m_pattern.isEmpty() || !m_mimeTypes.isEmpty() || !m_excludeMimeTypes.isEmpty() || !m_hiddenFilesShown);
}

bool KFileItemModelFilter::matches(const KFileItem &item) const
{
    // First check hidden files filtering
    if (!m_hiddenFilesShown && item.isHidden()) {
        // Hidden files are not being shown - check whitelist
        if (m_hiddenWhitelistEnabled && matchesHiddenWhitelist(item)) {
            // Item matches whitelist, continue to other filters
        } else {
            // Hidden and not whitelisted - filter it out
            return false;
        }
    }

    const bool hasPatternFilter = !m_pattern.isEmpty();
    const bool hasMimeTypesFilter = !m_mimeTypes.isEmpty() || !m_excludeMimeTypes.isEmpty();

    // If no filter is set, return true.
    if (!hasPatternFilter && !hasMimeTypesFilter) {
        return true;
    }

    // If both filters are set, return true when both filters are matched
    if (hasPatternFilter && hasMimeTypesFilter) {
        return (matchesPattern(item) && matchesType(item));
    }

    // If only one filter is set, return true when that filter is matched
    if (hasPatternFilter) {
        return matchesPattern(item);
    }

    return matchesType(item);
}

bool KFileItemModelFilter::matchesPattern(const KFileItem &item) const
{
    if (m_useRegExp) {
        return m_regExp->match(item.text()).hasMatch();
    } else {
        return item.text().toLower().contains(m_lowerCasePattern);
    }
}

bool KFileItemModelFilter::matchesType(const KFileItem &item) const
{
    bool excluded = std::any_of(m_excludeMimeTypes.constBegin(), m_excludeMimeTypes.constEnd(), [item](const QString &excludeMimetype) {
        return item.mimetype() == excludeMimetype;
    });
    if (excluded) {
        return false;
    }

    bool included = std::any_of(m_mimeTypes.constBegin(), m_mimeTypes.constEnd(), [item](const QString &mimeType) {
        return item.mimetype() == mimeType;
    });
    if (included) {
        return true;
    }

    return m_mimeTypes.isEmpty();
}
